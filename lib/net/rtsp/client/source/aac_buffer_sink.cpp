#include "aac_buffer_sink.h"
#include <H264VideoRTPSource.hh>

magnetar::lib::net::rtsp::client::aac_buffer_sink::aac_buffer_sink(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, unsigned buffer_size, int32_t channels, int32_t samplerate, char * configstr, int32_t configstr_size)
	: magnetar::lib::net::rtsp::client::buffer_sink(front, magnetar::lib::net::rtsp::client::media_type_t::audio, magnetar::lib::net::rtsp::client::audio_codec_type_t::aac, env, buffer_size)
	, _channels(channels)
	, _samplerate(samplerate)
	, _configstr_size(configstr_size)
{
	::memmove(_configstr, configstr, configstr_size);
	if (_front)
	{
		_front->_audio_channels = _channels;
		_front->_audio_samplerate = _samplerate;
		_front->_audio_extradata_size = _configstr_size;
		::memmove(_front->_audio_extradata, _configstr, _configstr_size);
	}
}

magnetar::lib::net::rtsp::client::aac_buffer_sink::~aac_buffer_sink(void)
{
}

magnetar::lib::net::rtsp::client::aac_buffer_sink* magnetar::lib::net::rtsp::client::aac_buffer_sink::createNew(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, unsigned buffer_size, int32_t channels, int32_t samplerate, char * configstr, int32_t configstr_size)
{
	return new aac_buffer_sink(front, env, buffer_size, channels, samplerate, configstr, configstr_size);
}


void magnetar::lib::net::rtsp::client::aac_buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec)
{
	magnetar::lib::net::rtsp::client::buffer_sink::after_getting_frame(frame_size, truncated_bytes, presentation_time, duration_msec);
}
