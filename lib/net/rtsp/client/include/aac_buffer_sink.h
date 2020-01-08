#if !defined(_AAC_BUFFER_SINK_H_)
#define _AAC_BUFFER_SINK_H_

#include "buffer_sink.h"
#include "mgnt_rtsp_client.h"

namespace magnetar
{
	namespace lib
	{
		namespace net
		{
			namespace rtsp
			{
				class client::aac_buffer_sink
					: public magnetar::lib::net::rtsp::client::buffer_sink
				{
				public:
					static magnetar::lib::net::rtsp::client::aac_buffer_sink * createNew(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, unsigned buffer_size, int32_t channels, int32_t samplerate, char * configstr, int32_t configstr_size);

				protected:
					aac_buffer_sink(magnetar::lib::net::rtsp::client * front, UsageEnvironment & env, unsigned buffer_size, int32_t channels, int32_t samplerate, char * configstr, int32_t configstr_size);
					virtual ~aac_buffer_sink(void);

					virtual void after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec);

				protected:
					int32_t _channels;
					int32_t _samplerate;
					char	_configstr[100];
					int32_t	_configstr_size;
				};
			};
		};
	};
};

#endif