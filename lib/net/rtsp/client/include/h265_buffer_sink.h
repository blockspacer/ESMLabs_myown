#if !defined(_H265_BUFFER_SINK_H_)
#define _H265_BUFFER_SINK_H_

#include "h2645_buffer_sink.h"
#include "mgnt_rtsp_client.h"

namespace magnetar
{
	namespace lib
	{
		namespace net
		{
			namespace rtsp
			{
				class client::h265_buffer_sink
					: public magnetar::lib::net::rtsp::client::h2645_buffer_sink
				{
				public:
					static magnetar::lib::net::rtsp::client::h265_buffer_sink * createNew(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);

				protected:
					h265_buffer_sink(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size);
					virtual ~h265_buffer_sink(void);
				};
			};
		};
	};
};

#endif // H265_BUFFER_SINK_H

