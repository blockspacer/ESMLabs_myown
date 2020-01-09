#include "h265_buffer_sink.h"

esmlabs::lib::net::rtsp::client::h265_buffer_sink::h265_buffer_sink(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: esmlabs::lib::net::rtsp::client::h2645_buffer_sink(front, esmlabs::lib::net::rtsp::client::video_codec_type_t::hevc, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size)
{

}

esmlabs::lib::net::rtsp::client::h265_buffer_sink::~h265_buffer_sink(void)
{

}

esmlabs::lib::net::rtsp::client::h265_buffer_sink * esmlabs::lib::net::rtsp::client::h265_buffer_sink::createNew(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
{
	return new h265_buffer_sink(front, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size);
}
