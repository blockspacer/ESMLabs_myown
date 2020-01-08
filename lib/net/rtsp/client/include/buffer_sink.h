#if !defined(_BUFFER_SINK_H_)
#define _BUFFER_SINK_H_

#include <MediaSink.hh>
#include <UsageEnvironment.hh>
#include "mgnt_rtsp_client.h"

namespace magnetar
{
	namespace lib
	{
		namespace net
		{
			namespace rtsp
			{
				class client::buffer_sink : public MediaSink
				{
				public:
					static magnetar::lib::net::rtsp::client::buffer_sink * createNew(magnetar::lib::net::rtsp::client * front, int32_t mt, int32_t codec, UsageEnvironment & env, unsigned buffer_size);

					virtual void add_data(unsigned char * data, unsigned size, struct timeval presentation_time, unsigned duration_msec);

				protected:
					buffer_sink(magnetar::lib::net::rtsp::client * front, int32_t mt, int32_t codec, UsageEnvironment & env, unsigned buffer_size);
					virtual ~buffer_sink(void);

				protected: //redefined virtual functions
					virtual Boolean continuePlaying(void);

				protected:
					static void after_getting_frame(void * param, unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec);
					virtual void after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec);


					magnetar::lib::net::rtsp::client * _front;
					unsigned char *	_buffer;
					unsigned		_buffer_size;

					bool	        _change_vps;
					bool	        _change_sps;
					bool	        _change_pps;
					bool	        _recv_first_idr;
					bool	        _recv_first_audio;

					struct timeval	_prev_presentation_time;
					unsigned		_same_presentation_time_counter;

					int32_t		    _mt;
					int32_t		    _vcodec;
					int32_t		    _acodec;
					long long	    _start_time;
				};
			};
		};
	};
};

#endif // BUFFER_SINK_H

