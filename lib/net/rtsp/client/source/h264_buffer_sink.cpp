#include "h264_buffer_sink.h"

magnetar::lib::net::rtsp::client::h264_buffer_sink::h264_buffer_sink(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: magnetar::lib::net::rtsp::client::h2645_buffer_sink(front, magnetar::lib::net::rtsp::client::video_codec_type_t::avc, env, nullptr, 0, sps, sps_size, pps, pps_size, buffer_size)
{

}

magnetar::lib::net::rtsp::client::h264_buffer_sink::~h264_buffer_sink(void)
{

}

magnetar::lib::net::rtsp::client::h264_buffer_sink* magnetar::lib::net::rtsp::client::h264_buffer_sink::createNew(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
{
	return new h264_buffer_sink(front, env, sps, sps_size, pps, pps_size, buffer_size);
}
