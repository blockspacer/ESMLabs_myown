#include "rtsp_client.h"
#include "esm_rtsp_client.h"
#include "h264_buffer_sink.h"
#include "h265_buffer_sink.h"
#include "aac_buffer_sink.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

esmlabs::lib::net::rtsp::client::core * esmlabs::lib::net::rtsp::client::core::createNew(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * url, const char * username, const char * password, int transport_option, int recv_option, int recv_timeout, float scale, unsigned int http_port_number, bool * kill_flag)
{
	return new esmlabs::lib::net::rtsp::client::core(front, env, url, username, password, transport_option, recv_option, recv_timeout, scale, http_port_number, kill_flag);
}

esmlabs::lib::net::rtsp::client::core::core(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * url, const char * username, const char * password, int transport_option, int recv_option, int recv_timeout, float scale, unsigned int http_port_number, bool * kill_flag)
	: RTSPClient(env, url, 1, "esmlabs::lib::net::rtsp::client", http_port_number, -1)
	, _front(front)
	, _kill_flag(kill_flag)
	, _kill_trigger(0)
    , _auth(0)
    , _media_session(0)
	, _iter(0)
    , _session_timer_task(0)
    , _arrival_check_timer_task(0)
    , _inter_packet_gap_check_timer_task(0)
    , _session_timeout_broken_server_task(0)
    , _socket_input_buffer_size(0)
    , _made_progress(false)
    , _session_timeout_parameter(60)
	, _inter_packet_gap_max_time(recv_timeout)
    , _total_packets_received(~0)
    , _duration(0.0)
    , _duration_slot(-1.0)
    , _init_seek_time(0.0f)
    , _init_abs_seek_time(0)
	, _scale(scale)
    , _end_time(0)
    , _shutting_down(false)
    , _wait_teardown_response(false)
    , _send_keepalives_to_broken_servers(true)
	, _sps(0)
	, _pps(0)
{
    _transport_option = transport_option;
    _recv_option = recv_option;

	_kill_trigger = envir().taskScheduler().createEventTrigger((TaskFunc*)&(esmlabs::lib::net::rtsp::client::core::kill_trigger));
	//_task_sched = &_env->taskScheduler();
	if (username && password && strlen(username)>0 && strlen(password)>0)
        _auth = new Authenticator(username, password);
}

esmlabs::lib::net::rtsp::client::core::~core(void)
{
	if (_auth)
	{
		delete _auth;
		_auth = 0;
	}
}

void esmlabs::lib::net::rtsp::client::core::get_options(RTSPClient::responseHandler * after_func)
{
    sendOptionsCommand(after_func, _auth);
}

void esmlabs::lib::net::rtsp::client::core::get_description(RTSPClient::responseHandler * after_func)
{
    sendDescribeCommand(after_func, _auth);
}

void esmlabs::lib::net::rtsp::client::core::setup_media_subsession(MediaSubsession * media_subsession, bool rtp_over_tcp, bool force_multicast_unspecified, RTSPClient::responseHandler * after_func)
{
    sendSetupCommand(*media_subsession, after_func, False, rtp_over_tcp?True:False, force_multicast_unspecified?True:False, _auth);
}

void esmlabs::lib::net::rtsp::client::core::start_playing_session(MediaSession * media_session, double start, double end, float scale, RTSPClient::responseHandler * after_func)
{
    sendPlayCommand(*media_session, after_func, start, end, scale, _auth);
}

void esmlabs::lib::net::rtsp::client::core::start_playing_session(MediaSession * media_session, const char * abs_start_time, const char * abs_end_time, float scale, RTSPClient::responseHandler * after_func)
{
    sendPlayCommand(*media_session, after_func, abs_start_time, abs_end_time, scale, _auth);
}

void esmlabs::lib::net::rtsp::client::core::start_pausing_session(void)
{
	sendPauseCommand(*_media_session, continue_after_pause, _auth);
}

void esmlabs::lib::net::rtsp::client::core::teardown_session(MediaSession * media_session, RTSPClient::responseHandler * after_func)
{
    sendTeardownCommand(*media_session, after_func, _auth);
}

void esmlabs::lib::net::rtsp::client::core::set_user_agent_string(const char * user_agent)
{
    setUserAgentString(user_agent);
}

void esmlabs::lib::net::rtsp::client::core::continue_after_client_creation(RTSPClient * param)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
    self->set_user_agent_string("esmlabs.AI RTSP Client");
    self->get_options(continue_after_options);
}

void esmlabs::lib::net::rtsp::client::core::continue_after_options(RTSPClient * param, int result_code, char * result_string)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
    delete [] result_string;
    self->get_description(continue_after_describe);
}

void esmlabs::lib::net::rtsp::client::core::continue_after_describe(RTSPClient * param, int result_code, char * result_string)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);

	do
	{
		if (result_code != 0)
		{
			delete[] result_string;
			break;
		}

		char * sdp_description = result_string;
		self->_media_session = MediaSession::createNew(self->envir(), sdp_description);
		delete[] sdp_description;

		if (!self->_media_session)
			break;
		if (!self->_media_session->hasSubsessions())
			break;

		MediaSubsessionIterator iter(*self->_media_session);
		MediaSubsession * media_subsession;

		bool made_progress = false;
		while ((media_subsession = iter.next()) != 0)
		{
			if ((self->_recv_option & esmlabs::lib::net::rtsp::client::media_type_t::audio) && !(self->_recv_option & esmlabs::lib::net::rtsp::client::media_type_t::video))
			{
				if (strcmp(media_subsession->mediumName(), "audio")!=0)
					continue;
			}
			if ((self->_recv_option & esmlabs::lib::net::rtsp::client::media_type_t::video) && !(self->_recv_option & esmlabs::lib::net::rtsp::client::media_type_t::audio))
			{
				if (strcmp(media_subsession->mediumName(), "video") != 0)
					continue;
			}
			if (media_subsession->initiate(-1))
			{
				//if( media_subsession->rtcpIsMuxed() )
				made_progress = true;
				if (media_subsession->rtpSource())
				{
					unsigned const threshold = 1000000; //1sec
					media_subsession->rtpSource()->setPacketReorderingThresholdTime(threshold);
					int fd = media_subsession->rtpSource()->RTPgs()->socketNum();
					if (self->_socket_input_buffer_size>0)
					{
						setReceiveBufferTo(self->envir(), fd, self->_socket_input_buffer_size);
					}
				}
			}
		}

		if (!made_progress)
			break;

		self->setup_streams();
		return;
	} while (0);

#if 0
	self->shutdown();
#else
	self->close();
#endif
}

void esmlabs::lib::net::rtsp::client::core::continue_after_setup(RTSPClient * param, int result_code, char * result_string)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);

    if( result_code==0 )
        self->_made_progress = true;

    delete [] result_string;

    self->_session_timeout_parameter = self->sessionTimeoutParameter();
    self->setup_streams();
}

void esmlabs::lib::net::rtsp::client::core::continue_after_play(RTSPClient * param, int result_code, char * result_string)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);

	do
	{
		if (result_code)
		{
			delete[] result_string;
			break;
		}
		delete[] result_string;

		// double delay_secs = self->_duration;
		if (self->_duration>0.0)
		{
			// First, adjust "duration" based on any change to the play range (that was specified in the "PLAY" response):
			double range_adjustment = (self->_media_session->playEndTime() - self->_media_session->playStartTime()) - (self->_end_time - self->_init_seek_time);
			if ((self->_duration + range_adjustment)>0.0)
			{
				self->_duration += range_adjustment;
			}

			double abs_scale = self->_scale>0 ? self->_scale : -(self->_scale);
			double delay_in_second = self->_duration / abs_scale + self->_duration_slot;

			long long delay_in_millisec = (long long)(delay_in_second*1000000.0);
			self->_session_timer_task = self->envir().taskScheduler().scheduleDelayedTask(delay_in_millisec, (TaskFunc*)session_timer_handler, (void*)0);
		}

		self->_session_timeout_broken_server_task = 0;
		//watch for incoming packet
		self->check_packet_arrival(self);

		if (self->_inter_packet_gap_max_time>0)
			self->check_inter_packet_gaps(self);
		self->check_session_timeout_broken_server(self);
		return;

	} while (0);

#if 0
	self->shutdown();
#else
	self->close();
#endif
}

void esmlabs::lib::net::rtsp::client::core::continue_after_pause(RTSPClient * param, int result_code, char * result_string)
{

}

void esmlabs::lib::net::rtsp::client::core::continue_after_teardown(RTSPClient * param, int result_code, char * result_string)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);

    if( result_string )
        delete [] result_string;

	if (!self->_media_session)
		return;

	MediaSubsessionIterator iter(*self->_media_session);
	MediaSubsession * media_subsession;
	while ((media_subsession = iter.next()) != 0)
	{
		Medium::close(media_subsession->sink);
		media_subsession->sink = 0;
	}

    Medium::close( self->_media_session );
	
	self->_shutting_down = false;
	self->close();
}

void esmlabs::lib::net::rtsp::client::core::close(void)
{
	//envir().taskScheduler();
	envir().taskScheduler().triggerEvent(_kill_trigger, this);
}

void esmlabs::lib::net::rtsp::client::core::subsession_after_playing(void * param)
{
	MediaSubsession * media_subsession = static_cast<MediaSubsession*>(param);
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(media_subsession->miscPtr);

    Medium::close( media_subsession->sink );
    media_subsession->sink = 0;

    MediaSession & media_session = media_subsession->parentSession();
    MediaSubsessionIterator iter( media_session );
    while( (media_subsession=iter.next())!=0 )
    {
        if( media_subsession->sink ) //this subsession is still active
            return;
    }
	self->session_after_playing(self);
}

void esmlabs::lib::net::rtsp::client::core::subsession_bye_handler(void * param)
{
	MediaSubsession * media_subsession = static_cast<MediaSubsession*>(param);
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(media_subsession->miscPtr);

    //struct timeval time_now;
    //gettimeofday( &time_now, 0 );
    //unsigned diff_secs = time_now.tv_sec - start_time.tv_sec;
	self->subsession_after_playing(media_subsession);
}

void esmlabs::lib::net::rtsp::client::core::session_after_playing(void * param)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
#if 0
	self->shutdown();
#else
	self->close();
#endif
}

void esmlabs::lib::net::rtsp::client::core::setup_streams(void)
{
	//static MediaSubsessionIterator * iter = new MediaSubsessionIterator(*_media_session);
	if (!_iter)
		_iter = new MediaSubsessionIterator(*_media_session);
    MediaSubsession * media_subsession = 0;
	while ((media_subsession = _iter->next()) != 0)
    {
        if( media_subsession->clientPortNum()==0 )
            continue;
        if(_transport_option== esmlabs::lib::net::rtsp::client::transport_option_t::rtp_over_tcp)
			setup_media_subsession(media_subsession, true, false, esmlabs::lib::net::rtsp::client::core::continue_after_setup);
        else
			setup_media_subsession(media_subsession, false, false, esmlabs::lib::net::rtsp::client::core::continue_after_setup);

		return;
    }

	if (_iter)
	{
		delete _iter;
		_iter = 0;
	}

	if(!_made_progress)
	{
		shutdown();
		return;
	}

	_made_progress = false;
	MediaSubsessionIterator iter2(*_media_session);
	while((media_subsession=iter2.next())!=0)
	{
		if(media_subsession->readSource()==0) // was not initiated
			continue;

		esmlabs::lib::net::rtsp::client::buffer_sink * bs = nullptr;
		if (!strcmp(media_subsession->mediumName(), "video"))
		{
			if(!strcmp(media_subsession->codecName(), "H265"))
			{
				const char * vps = media_subsession->fmtp_spropvps();
				const char * sps = media_subsession->fmtp_spropsps();
				const char * pps = media_subsession->fmtp_sproppps();
				bs = h265_buffer_sink::createNew(_front, envir(), vps, uint32_t(strlen(vps)), sps, uint32_t(strlen(sps)), pps, uint32_t(strlen(pps)), 8 * 1024 * 1024);
			}
			else if (!strcmp(media_subsession->codecName(), "H264"))
			{
				uint8_t * spspps[2] = { 0 };
				int32_t spspps_size[2] = { 0 };
				uint32_t num_spspps = 0;
				SPropRecord * sPropRecord = parseSPropParameterSets(media_subsession->fmtp_spropparametersets(), num_spspps);
				for (int32_t i = 0; i < num_spspps; i++)
				{
					spspps[i] = sPropRecord[i].sPropBytes;
					spspps_size[i] = sPropRecord[i].sPropLength;
				}

				bs = h264_buffer_sink::createNew(_front, envir(), (const char*)spspps[0], spspps_size[0], (const char*)spspps[1], spspps_size[1], 4 * 1024 * 1024);
				delete[] sPropRecord;
			}
			else if (!strcmp(media_subsession->codecName(), "MP4V-ES"))
			{

			}
		}
		else if(!strcmp(media_subsession->mediumName(), "audio"))
		{
			if (!strcmp(media_subsession->codecName(), "AMR") || !strcmp(media_subsession->codecName(), "AMR-WB"))
			{
				//bs = amr_buffer_sink::createNew(_front, envir(), media_subsession->fmtp_spropparametersets(), 512 * 1024);
			}
			else if (!strcmp(media_subsession->codecName(), "MPEG4-GENERIC") || !strcmp(media_subsession->codecName(), "MP4A-LATM")) //aac
			{
				int32_t frequencyFromconfig = (int32_t)samplingFrequencyFromAudioSpecificConfig(media_subsession->fmtp_config());
				char * configStr = (char*)media_subsession->fmtp_config();
				bs = aac_buffer_sink::createNew(_front, envir(), 512 * 1024, media_subsession->numChannels(), frequencyFromconfig, configStr, int32_t(strlen(configStr)));
			}
		}

		media_subsession->sink = bs;
		if(media_subsession->sink)
		{
			media_subsession->miscPtr = (void*)this;
			media_subsession->sink->startPlaying(*(media_subsession->readSource()), subsession_after_playing, media_subsession);

			// also set a handler to be called if a RTCP "BYE" arrives
			// for this subsession:

			if (media_subsession->rtcpInstance() != NULL)
				media_subsession->rtcpInstance()->setByeHandler(subsession_bye_handler, media_subsession);
			_made_progress = true;
		}
	}

    if( _duration==0 )
    {
        if( _scale>0 )
            _duration = _media_session->playEndTime() - _init_seek_time;
        else if( _scale<0 )
            _duration = _init_seek_time;
    }
    if( _duration<0 )
        _duration = 0.0;

    _end_time = _init_seek_time;
    if( _scale>0 )
    {
        if( _duration<=0 )
            _end_time = -1.0f;
        else
            _end_time = _init_seek_time + _duration;
    }
    else
    {
        _end_time = _init_seek_time - _duration;
        if( _end_time<0 )
            _end_time = 0.0f;
    }

    const char * abs_start_time = _init_abs_seek_time?_init_abs_seek_time:_media_session->absStartTime();
    if( abs_start_time )
        start_playing_session( _media_session, abs_start_time, _media_session->absEndTime(), _scale, continue_after_play );
    else
        start_playing_session( _media_session, _init_seek_time, _end_time, _scale, continue_after_play );
}

void esmlabs::lib::net::rtsp::client::core::shutdown(void)
{
    if( _shutting_down )
        return;

    _shutting_down = true;

    envir().taskScheduler().unscheduleDelayedTask(_session_timer_task);
	envir().taskScheduler().unscheduleDelayedTask(_session_timeout_broken_server_task);
	envir().taskScheduler().unscheduleDelayedTask(_arrival_check_timer_task);
	envir().taskScheduler().unscheduleDelayedTask(_inter_packet_gap_check_timer_task);

    bool shutdown_immediately = true;
    if( _media_session )
    {
        RTSPClient::responseHandler * response_handler_for_teardown = 0;
        if( _wait_teardown_response )
        {
            shutdown_immediately = false;
            response_handler_for_teardown = continue_after_teardown;
        }
        teardown_session( _media_session, response_handler_for_teardown );
    }

    if( shutdown_immediately )
        continue_after_teardown( this, 0, 0 );
}

void esmlabs::lib::net::rtsp::client::core::session_timer_handler(void * param)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
    self->_session_timer_task = 0;
	self->session_after_playing(self);
}

void esmlabs::lib::net::rtsp::client::core::check_packet_arrival(void * param)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
    int delay_usec = 100000; //100 ms
	self->_arrival_check_timer_task = self->envir().taskScheduler().scheduleDelayedTask(delay_usec, (TaskFunc*)&esmlabs::lib::net::rtsp::client::core::check_packet_arrival, self);
}

void esmlabs::lib::net::rtsp::client::core::check_inter_packet_gaps(void * param)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
    if( !self->_inter_packet_gap_max_time )
        return;

    unsigned total_packets_received = 0;

    MediaSubsessionIterator iter( *self->_media_session );
    MediaSubsession * media_subsession;
    while( (media_subsession=iter.next())!=0 )
    {
        RTPSource * src = media_subsession->rtpSource();
        if( !src )
            continue;

        total_packets_received += src->receptionStatsDB().totNumPacketsReceived();
    }

    if( total_packets_received==self->_total_packets_received )
    {
        self->_inter_packet_gap_check_timer_task = 0;
        self->session_after_playing(self);
    }
    else
    {
        self->_total_packets_received = total_packets_received;
		self->_inter_packet_gap_check_timer_task = self->envir().taskScheduler().scheduleDelayedTask(self->_inter_packet_gap_max_time * 1000000, (TaskFunc*)&esmlabs::lib::net::rtsp::client::core::check_inter_packet_gaps, self);
    }
}

void esmlabs::lib::net::rtsp::client::core::check_session_timeout_broken_server(void * param)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
    if( !self->_send_keepalives_to_broken_servers )
        return;

    if( self->_session_timeout_broken_server_task )
        self->get_options( 0 );

    unsigned session_timeout = self->_session_timeout_parameter;
    unsigned delay_sec_until_next_keepalive = session_timeout<=5?1:session_timeout-5;
    //reduce the interval a liteel, to be on the safe side

	self->_session_timeout_broken_server_task = self->envir().taskScheduler().scheduleDelayedTask(delay_sec_until_next_keepalive * 1000000, (TaskFunc*)&esmlabs::lib::net::rtsp::client::core::check_session_timeout_broken_server, self);
}

void esmlabs::lib::net::rtsp::client::core::kill_trigger(void * param)
{
	esmlabs::lib::net::rtsp::client::core * self = static_cast<esmlabs::lib::net::rtsp::client::core*>(param);
	self->_shutting_down = false;
	self->shutdown();

	if (self->_kill_flag)
		(*self->_kill_flag) = true;

	Medium::close(self);
}