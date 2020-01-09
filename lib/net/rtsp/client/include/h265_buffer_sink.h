#ifndef _H265_BUFFER_SINK_H_
#define _H265_BUFFER_SINK_H_

#include "h2645_buffer_sink.h"
#include "esm_rtsp_client.h"

namespace esmlabs
{
	namespace lib
	{
		namespace net
		{
			namespace rtsp
			{
				class client::h265_buffer_sink
					: public esmlabs::lib::net::rtsp::client::h2645_buffer_sink
				{
				public:
					static esmlabs::lib::net::rtsp::client::h265_buffer_sink * createNew(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);

				protected:
					h265_buffer_sink(esmlabs::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);
					virtual ~h265_buffer_sink(void);
				};
			};
		};
	};
};

#endif // H265_BUFFER_SINK_H

