#include "h264_buffer_sink.h"

esmlabs::lib::net::rtsp::client::h264_buffer_sink::h264_buffer_sink(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: esmlabs::lib::net::rtsp::client::h2645_buffer_sink(front, esmlabs::lib::net::rtsp::client::video_codec_type_t::avc, env, nullptr, 0, sps, sps_size, pps, pps_size, buffer_size)
{

}

esmlabs::lib::net::rtsp::client::h264_buffer_sink::~h264_buffer_sink(void)
{

}

esmlabs::lib::net::rtsp::client::h264_buffer_sink* esmlabs::lib::net::rtsp::client::h264_buffer_sink::createNew(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
{
	return new h264_buffer_sink(front, env, sps, sps_size, pps, pps_size, buffer_size);
}
