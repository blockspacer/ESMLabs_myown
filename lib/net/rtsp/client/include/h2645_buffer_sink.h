#ifndef _H2645_BUFFER_SINK_H_
#define _H2645_BUFFER_SINK_H_

#include "buffer_sink.h"
#include "esm_rtsp_client.h"

namespace esmlabs
{
	namespace lib
	{
		namespace net
		{
			namespace rtsp
			{
				class client::h2645_buffer_sink
					: public esmlabs::lib::net::rtsp::client::buffer_sink
				{
				public:
					h2645_buffer_sink(esmlabs::lib::net::rtsp::client * front, int32_t codec, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);
					virtual ~h2645_buffer_sink(void);

				protected:
					virtual void after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec);

				private:
					const char *	_vspps[3];
					bool			_receive_first_frame;
					unsigned char	_vspps_buffer[MAX_PATH];
				};
			};
		};
	};
};
#endif // H264_BUFFER_SINK_H

