#include "h2645_buffer_sink.h"
#include <H264VideoRTPSource.hh>

magnetar::lib::net::rtsp::client::h2645_buffer_sink::h2645_buffer_sink(magnetar::lib::net::rtsp::client * front, int32_t codec, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: magnetar::lib::net::rtsp::client::buffer_sink(front, magnetar::lib::net::rtsp::client::media_type_t::video, codec, env, buffer_size)
	, _receive_first_frame(false)
{
	if (vps != nullptr && vps_size > 0)
		_vspps[0] = ::strdup(vps);
	else
		_vspps[0] = nullptr;
	if (sps != nullptr && sps_size > 0)
		_vspps[1] = ::strdup(sps);
	else
		_vspps[1] = nullptr;
	if (pps != nullptr && pps_size > 0)
		_vspps[2] = ::strdup(pps);
	else
		_vspps[2] = nullptr;
	::memset(_vspps_buffer, 0x00, sizeof(_vspps_buffer));
}

magnetar::lib::net::rtsp::client::h2645_buffer_sink::~h2645_buffer_sink(void)
{
	if (_vspps[0] != nullptr)
		::free((void*)_vspps[0]);
	if (_vspps[1] != nullptr)
		::free((void*)_vspps[1]);
	if (_vspps[2] != nullptr)
		::free((void*)_vspps[2]);
}

void magnetar::lib::net::rtsp::client::h2645_buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec)
{
    const unsigned char start_code[4] = {0x00, 0x00, 0x00, 0x01};
	//if (!_front->ignore_sdp())
	{
		if (!_receive_first_frame)
		{
			for (unsigned i = 0; i < 3; ++i)
			{
				unsigned number_of_vspspps_records;
				SPropRecord * spspps_records = parseSPropParameterSets(_vspps[i], number_of_vspspps_records);
				for (unsigned j = 0; j < number_of_vspspps_records; ++j)
				{
					::memmove(_vspps_buffer, start_code, sizeof(start_code));
					::memmove(_vspps_buffer + sizeof(start_code), spspps_records[j].sPropBytes, spspps_records[j].sPropLength);
					add_data(_vspps_buffer, sizeof(start_code) + spspps_records[j].sPropLength, presentation_time, duration_msec);
				}
				delete[] spspps_records;
			}
			_receive_first_frame = true;
		}
    }
    buffer_sink::after_getting_frame(frame_size, truncated_bytes, presentation_time, duration_msec);
}
