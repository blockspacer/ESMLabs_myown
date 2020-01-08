#include "h265_buffer_sink.h"

magnetar::lib::net::rtsp::client::h265_buffer_sink::h265_buffer_sink(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: magnetar::lib::net::rtsp::client::h2645_buffer_sink(front, magnetar::lib::net::rtsp::client::video_codec_type_t::hevc, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size)
{

}

magnetar::lib::net::rtsp::client::h265_buffer_sink::~h265_buffer_sink(void)
{

}

magnetar::lib::net::rtsp::client::h265_buffer_sink * magnetar::lib::net::rtsp::client::h265_buffer_sink::createNew(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
{
	return new h265_buffer_sink(front, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size);
}
