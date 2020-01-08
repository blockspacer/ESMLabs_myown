#include "mgnt_rtsp_client.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "rtsp_client.h"

#define AV_RB32(x)  \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) | \
               (((const uint8_t*)(x))[1] << 16) | \
               (((const uint8_t*)(x))[2] <<  8) | \
                ((const uint8_t*)(x))[3])

#define MIN(a,b) ((a) > (b) ? (b) : (a))

magnetar::lib::net::rtsp::client::client(void)
	: _ignore_sdp(true)
	, _tid(-1)
{
}

magnetar::lib::net::rtsp::client::~client(void)
{
}

int32_t magnetar::lib::net::rtsp::client::play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, int32_t recv_timeout, float scale, bool repeat)
{
    if( !url || strlen(url)<1 )
		return magnetar::lib::net::rtsp::client::err_code_t::generic_fail;

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	if (strlen(url)>0)
		strncpy(_url, url, sizeof(_url));
	if (username && strlen(username)>0)
		strncpy(_username, username, sizeof(_username));
	if (password && strlen(password)>0)
		strncpy(_password, password, sizeof(_password));
	_transport_option = transport_option;
	_recv_option = recv_option;
	_recv_timeout = recv_timeout;
	_scale = scale;
	_repeat = repeat;

	_tid = pthread_create(&_thread, nullptr, magnetar::lib::net::rtsp::client::process_cb, (void*)this);
	return magnetar::lib::net::rtsp::client::err_code_t::success;
}

int32_t magnetar::lib::net::rtsp::client::stop(void)
{
	if (!_kill )
	{
		_repeat = false;
		_kill = true;
		if (_live)
			_live->close();
	}
	pthread_join(_thread, nullptr);
	return magnetar::lib::net::rtsp::client::err_code_t::success;
}

int32_t magnetar::lib::net::rtsp::client::pause(void)
{
	if (!_kill)
	{
		if (_live)
		{
			_live->start_pausing_session();
		}
	}
	return magnetar::lib::net::rtsp::client::err_code_t::success;
}

uint8_t * magnetar::lib::net::rtsp::client::get_vps(int32_t & vps_size)
{
	vps_size = _vps_size;
	return _sps;
}

uint8_t * magnetar::lib::net::rtsp::client::get_sps(int32_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * magnetar::lib::net::rtsp::client::get_pps(int32_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

void magnetar::lib::net::rtsp::client::set_vps(uint8_t * vps, int32_t vps_size)
{
	::memset(_vps, 0x00, sizeof(_vps));
	::memmove(_vps, vps, vps_size);
	_vps_size = vps_size;
}

void magnetar::lib::net::rtsp::client::set_sps(uint8_t * sps, int32_t sps_size)
{
	::memset(_sps, 0x00, sizeof(_sps));
	::memmove(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void magnetar::lib::net::rtsp::client::set_pps(uint8_t * pps, int32_t pps_size)
{
	::memset(_pps, 0x00, sizeof(_pps));
	::memmove(_pps, pps, pps_size);
	_pps_size = pps_size;
}


void magnetar::lib::net::rtsp::client::set_audio_extradata(uint8_t * extradata, int32_t size)
{
	::memset(_audio_extradata, 0x00, sizeof(_audio_extradata));
	::memmove(_audio_extradata, extradata, size);
	_audio_extradata_size = size;
}

uint8_t * magnetar::lib::net::rtsp::client::get_audio_extradata(int32_t & size)
{
	size = _audio_extradata_size;
	return _audio_extradata;
}

void magnetar::lib::net::rtsp::client::set_audio_channels(int32_t channels)
{
	_audio_channels = channels;
}

int32_t	magnetar::lib::net::rtsp::client::get_audio_channels(void)
{
	return _audio_channels;
}

void magnetar::lib::net::rtsp::client::set_audio_samplerate(int32_t samplerate)
{
	_audio_samplerate = samplerate;
}

int32_t magnetar::lib::net::rtsp::client::get_audio_samplerate(void)
{
	return _audio_samplerate;
}

/*
bool magnetar::lib::net::rtsp::client::ignore_sdp(void)
{
	return _ignore_sdp;
}
*/

void magnetar::lib::net::rtsp::client::on_begin_video(int32_t smt, uint8_t * vps, int32_t vpssize, uint8_t * sps, int32_t spssize, uint8_t * pps, int32_t ppssize, const uint8_t * bytes, int32_t nbytes, long long pts)
{

}

void magnetar::lib::net::rtsp::client::on_recv_video(int32_t smt, const uint8_t * bytes, int32_t nbytes, long long pts)
{

}

void magnetar::lib::net::rtsp::client::on_end_video(void)
{

}

void magnetar::lib::net::rtsp::client::on_begin_audio(int32_t smt, uint8_t * config, int32_t config_size, int32_t samplerate, int32_t channels, const uint8_t * bytes, int32_t nbytes, long long pts)
{

}

void magnetar::lib::net::rtsp::client::on_recv_audio(int32_t smt, const uint8_t * bytes, int32_t nbytes, long long pts)
{

}

void magnetar::lib::net::rtsp::client::on_end_audio(void)
{

}

void * magnetar::lib::net::rtsp::client::process_cb(void * param)
{
	magnetar::lib::net::rtsp::client * self = static_cast<magnetar::lib::net::rtsp::client*>(param);
	self->process();
	return 0;
}

void magnetar::lib::net::rtsp::client::process(void)
{
	do
	{
		TaskScheduler * sched = BasicTaskScheduler::createNew();
		UsageEnvironment * env = BasicUsageEnvironment::createNew(*sched);
		if (strlen(_username) > 0 && strlen(_password) > 0)
			_live = magnetar::lib::net::rtsp::client::core::createNew(this, *env, _url, _username, _password, _transport_option, _recv_option, _recv_timeout, _scale, 0, &_kill);
		else
			_live = magnetar::lib::net::rtsp::client::core::createNew(this, *env, _url, 0, 0, _transport_option, _recv_option, _recv_timeout, _scale, 0, &_kill);

		_kill = false;
		magnetar::lib::net::rtsp::client::core::continue_after_client_creation(_live);
		env->taskScheduler().doEventLoop((char*)&_kill);

		if (env)
		{
			env->reclaim();
			env = 0;
		}
		if (sched)
		{
			delete sched;
			sched = 0;
		}

		_sps_size = 0;
		_pps_size = 0;
		memset(_sps, 0x00, sizeof(_sps));
		memset(_pps, 0x00, sizeof(_pps));
	} while (_repeat);

	on_end_video();
	on_end_audio();
}

bool magnetar::lib::net::rtsp::client::is_vps(uint8_t nal_unit_type)
{
	return nal_unit_type == 32;
}

bool magnetar::lib::net::rtsp::client::is_sps(int32_t codec, uint8_t nal_unit_type)
{
	if (codec == magnetar::lib::net::rtsp::client::video_codec_type_t::avc)
		return nal_unit_type == 7;
	else
		return nal_unit_type == 33;
}

bool magnetar::lib::net::rtsp::client::is_pps(int32_t codec, uint8_t nal_unit_type)
{
	if (codec == magnetar::lib::net::rtsp::client::video_codec_type_t::avc)
		return nal_unit_type == 8;
	else
		return nal_unit_type == 34;
}

bool magnetar::lib::net::rtsp::client::is_idr(int32_t codec, uint8_t nal_unit_type)
{
	if (codec == magnetar::lib::net::rtsp::client::video_codec_type_t::avc)
		return nal_unit_type == 5;
	else
		return (nal_unit_type == 19) || (nal_unit_type == 20);
}

bool magnetar::lib::net::rtsp::client::is_vlc(int32_t codec, uint8_t nal_unit_type)
{
	if (codec == magnetar::lib::net::rtsp::client::video_codec_type_t::avc)
		return (nal_unit_type <= 5 && nal_unit_type > 0);
	else
		return (nal_unit_type <= 31);
}

const int32_t magnetar::lib::net::rtsp::client::find_nal_unit(uint8_t * bitstream, int32_t size, int * nal_start, int * nal_end)
{
	uint32_t i;
	// find start
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	//( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0 || bitstream[i + 3] != 0x01))
	{
		i++; // skip leading zero
		if (i + 4 >= size)
		{
			return 0;
		} // did not find nal start
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
	{
		i++;
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
	{
		/* error, should never happen */
		return 0;
	}

	i += 3;
	*nal_start = i;

	//( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01))
	{
		i++;
		// FIXME the next line fails when reading a nal that ends exactly at the end of the data
		if (i + 3 >= size)
		{
			*nal_end = size;
			return -1;
		} // did not find nal end, stream ended first
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}


const uint8_t * magnetar::lib::net::rtsp::client::find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state)
{
	int i;
	if (begin >= end)
		return end;

	for (i = 0; i < 3; i++)
	{
		uint32_t tmp = *state << 8;
		*state = tmp + *(begin++);
		if (tmp == 0x100 || begin == end)
			return begin;
	}

	while (begin < end)
	{
		if (begin[-1] > 1)
			begin += 3;
		else if (begin[-2])
			begin += 2;
		else if (begin[-3] | (begin[-1] - 1))
			begin++;
		else
		{
			begin++;
			break;
		}
	}

	begin = MIN(begin, end) - 4;
	*state = AV_RB32(begin);
	return begin + 4;
}