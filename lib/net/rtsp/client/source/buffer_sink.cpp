#include "buffer_sink.h"
#include <GroupsockHelper.hh>

esmlabs::lib::net::rtsp::client::buffer_sink::buffer_sink(esmlabs::lib::net::rtsp::client * front, int32_t mt, int32_t codec, UsageEnvironment & env, unsigned buffer_size)
    : MediaSink(env)
	, _front(front)
    , _buffer_size(buffer_size)
    , _same_presentation_time_counter(0)
	, _mt(mt)
	, _change_sps(false)
	, _change_pps(false)
	, _recv_first_idr(false)
	, _recv_first_audio(false)
{
	if (_mt == esmlabs::lib::net::rtsp::client::media_type_t::video)
		_vcodec = codec;
	if (_mt == esmlabs::lib::net::rtsp::client::media_type_t::audio)
		_acodec = codec;

    _buffer = new unsigned char[buffer_size];
    //_prev_presentation_time.tv_sec = ~0;
    //_prev_presentation_time.tv_usec = ~0;

	_start_time = 0;
}

esmlabs::lib::net::rtsp::client::buffer_sink::~buffer_sink(void)
{
	if (_buffer)
	{
		delete[] _buffer;
		_buffer = 0;
	}
}

esmlabs::lib::net::rtsp::client::buffer_sink* esmlabs::lib::net::rtsp::client::buffer_sink::createNew(esmlabs::lib::net::rtsp::client * front, int32_t mt, int32_t codec, UsageEnvironment & env, unsigned buffer_size)
{
	return new buffer_sink(front, mt, codec, env, buffer_size);
}

Boolean esmlabs::lib::net::rtsp::client::buffer_sink::continuePlaying(void)
{
    if( !fSource )
        return False;

    fSource->getNextFrame(_buffer, _buffer_size, after_getting_frame, this, onSourceClosure, this);
    return True;
}

void esmlabs::lib::net::rtsp::client::buffer_sink::after_getting_frame(void * param, unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec)
{
	esmlabs::lib::net::rtsp::client::buffer_sink * sink = static_cast<esmlabs::lib::net::rtsp::client::buffer_sink*>(param);
	sink->after_getting_frame(frame_size, truncated_bytes, presentation_time, duration_msec);
}

void esmlabs::lib::net::rtsp::client::buffer_sink::add_data(unsigned char * data, unsigned data_size, struct timeval presentation_time, unsigned duration_msec)
{
	long long pts = 0;
	if (!_start_time)
	{
		_start_time = (presentation_time.tv_sec * (uint64_t)1000) + (presentation_time.tv_usec / 1000);
	}
	else
	{
		pts = (presentation_time.tv_sec * (uint64_t)1000) + (presentation_time.tv_usec / 1000);
		pts -= _start_time;
	}
	
	if(_front)
	{
		if (_mt == esmlabs::lib::net::rtsp::client::media_type_t::video)
		{
			if (_vcodec == esmlabs::lib::net::rtsp::client::video_codec_type_t::avc)
			{
				int32_t saved_sps_size = 0;
				unsigned char * saved_sps = _front->get_sps(saved_sps_size);
				int32_t saved_pps_size = 0;
				unsigned char * saved_pps = _front->get_pps(saved_pps_size);

				bool is_sps = esmlabs::lib::net::rtsp::client::is_sps(_vcodec, data[4] & 0x1F);
				if (is_sps)
				{
					if (saved_sps_size < 1 || !saved_sps)
					{
						_front->set_sps(data, data_size);
						_change_sps = true;
						return;
					}
					else
					{
						if (::memcmp(saved_sps, data, saved_sps_size))
						{
							_front->set_sps(data, data_size);
							_change_sps = true;
							return;
						}
					}
				}

				bool is_pps = esmlabs::lib::net::rtsp::client::is_pps(_vcodec, data[4] & 0x1F);
				if (is_pps)
				{
					if (saved_pps_size < 1 || !saved_pps)
					{
						_front->set_pps(data, data_size);
						_change_pps = true;
						return;
					}
					else
					{
						if (::memcmp(saved_pps, data, saved_pps_size))
						{
							_front->set_pps(data, data_size);
							_change_pps = true;
							return;
						}
					}
				}

				bool is_idr = esmlabs::lib::net::rtsp::client::is_idr(_vcodec, data[4] & 0x1F);
				if (_change_sps || _change_pps)
				{
					if (is_idr && !_recv_first_idr)
					{
						_front->on_begin_video(_vcodec, nullptr, 0, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, pts);

						_recv_first_idr = true;
						_change_sps = false;
						_change_pps = false;
						return;
					}
				}
				else
				{
					if (is_idr && !_recv_first_idr)
					{
						_front->on_begin_video(_vcodec, nullptr, 0, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, pts);

						_recv_first_idr = true;
						return;
					}
				}

				if (_recv_first_idr)
				{
					_front->on_recv_video(_vcodec, data, data_size, pts);
					return;
				}
				/*
				if (!is_sps && !is_pps && _recv_idr)
				{
					saved_sps = _front->get_sps(saved_sps_size);
					saved_pps = _front->get_pps(saved_pps_size);
					if (saved_sps_size > 0 && saved_pps_size > 0)
					{
						_front->on_recv_video(_vsmt, data, data_size, dts, cts);
					}
				}
				*/
			}
			else if (_vcodec == esmlabs::lib::net::rtsp::client::video_codec_type_t::hevc)
			{
				int32_t saved_vps_size = 0;
				int32_t saved_sps_size = 0;
				int32_t saved_pps_size = 0;
				unsigned char * saved_vps = _front->get_vps(saved_vps_size);
				unsigned char * saved_sps = _front->get_sps(saved_sps_size);
				unsigned char * saved_pps = _front->get_pps(saved_pps_size);

				bool is_vps = esmlabs::lib::net::rtsp::client::is_vps((data[4] >> 1) & 0x3F);
				if (is_vps)
				{
					if (saved_vps_size < 1 || !saved_vps)
					{
						_front->set_vps(data, data_size);
						_change_vps = true;
						return;
					}
					else
					{
						if (::memcmp(saved_vps, data, saved_vps_size))
						{
							_front->set_vps(data, data_size);
							_change_vps = true;
							return;
						}
					}
				}

				bool is_sps = esmlabs::lib::net::rtsp::client::is_sps(_vcodec, (data[4] >> 1) & 0x3F);
				if (is_sps)
				{
					if (saved_sps_size < 1 || !saved_sps)
					{
						_front->set_sps(data, data_size);
						_change_sps = true;
						return;
					}
					else
					{
						if (::memcmp(saved_sps, data, saved_sps_size))
						{
							_front->set_sps(data, data_size);
							_change_sps = true;
							return;
						}
					}
				}

				bool is_pps = esmlabs::lib::net::rtsp::client::is_pps(_vcodec, (data[4] >> 1) & 0x3F);
				if (is_pps)
				{
					if (saved_pps_size < 1 || !saved_pps)
					{
						_front->set_pps(data, data_size);
						_change_pps = true;
						return;
					}
					else
					{
						if (::memcmp(saved_pps, data, saved_pps_size))
						{
							_front->set_pps(data, data_size);
							_change_pps = true;
							return;
						}
					}
				}

				bool is_idr = esmlabs::lib::net::rtsp::client::is_idr(_vcodec, (data[4] >> 1) & 0x3F);
				if (_change_vps || _change_sps || _change_pps)
				{
					if (is_idr && !_recv_first_idr)
					{
						_front->on_begin_video(_vcodec, saved_vps, saved_vps_size, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, pts);

						_recv_first_idr = true;
						_change_vps = false;
						_change_sps = false;
						_change_pps = false;
						return;
					}
				}
				else
				{
					if (is_idr && !_recv_first_idr)
					{
						_front->on_begin_video(_vcodec, saved_vps, saved_vps_size, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, pts);

						_recv_first_idr = true;
						return;
					}
				}

				if (_recv_first_idr)
				{
					_front->on_recv_video(_vcodec, data, data_size, pts);
					return;
				}
			}
		}		
		else if (_mt == esmlabs::lib::net::rtsp::client::media_type_t::audio)
		{
			if (_acodec == esmlabs::lib::net::rtsp::client::audio_codec_type_t::aac)
			{
				if (!_recv_first_audio)
				{
					int32_t extradata_size = 0;
					uint8_t * extradata = _front->get_audio_extradata(extradata_size);
					int32_t channels = _front->get_audio_channels();
					int32_t samplerate = _front->get_audio_samplerate();
					_front->on_begin_audio(_acodec, extradata, extradata_size, samplerate, channels, data, data_size, pts);
					_recv_first_audio = true;
				}
				else
				{
					_front->on_recv_audio(_acodec, data, data_size, pts);
				}
			}
		}
	}
}

void esmlabs::lib::net::rtsp::client::buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_msec)
{
	if (_front)
	{
		if (_mt == esmlabs::lib::net::rtsp::client::media_type_t::video)
		{
			if (_vcodec == esmlabs::lib::net::rtsp::client::video_codec_type_t::avc)
			{
				const unsigned char start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
				if ((_buffer[0] == start_code[0]) && (_buffer[1] == start_code[1]) && (_buffer[2] == start_code[2]) && (_buffer[3] == start_code[3]))
					add_data(_buffer, frame_size, presentation_time, duration_msec);
				else
				{
					if (truncated_bytes > 0)
						::memmove(_buffer + 4, _buffer, frame_size - 4);
					else
					{
						truncated_bytes = (frame_size + 4) - _buffer_size;
						if (truncated_bytes > 0 && (frame_size + 4) > _buffer_size)
							::memmove(_buffer + 4, _buffer, frame_size - truncated_bytes);
						else
							::memmove(_buffer + 4, _buffer, frame_size);
					}
					::memmove(_buffer, start_code, sizeof(start_code));
					add_data(_buffer, frame_size + sizeof(start_code), presentation_time, duration_msec);
				}
			} 
			else if (_vcodec == esmlabs::lib::net::rtsp::client::video_codec_type_t::hevc)
			{
				const unsigned char start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
				if ((_buffer[0] == start_code[0]) && (_buffer[1] == start_code[1]) && (_buffer[2] == start_code[2]) && (_buffer[3] == start_code[3]))
					add_data(_buffer, frame_size, presentation_time, duration_msec);
				else
				{
					if (truncated_bytes > 0)
						::memmove(_buffer + 4, _buffer, frame_size - 4);
					else
					{
						truncated_bytes = (frame_size + 4) - _buffer_size;
						if (truncated_bytes > 0 && (frame_size + 4) > _buffer_size)
							::memmove(_buffer + 4, _buffer, frame_size - truncated_bytes);
						else
							::memmove(_buffer + 4, _buffer, frame_size);
					}
					::memmove(_buffer, start_code, sizeof(start_code));
					add_data(_buffer, frame_size + sizeof(start_code), presentation_time, duration_msec);
				}
			}
		}
		else if (_mt == esmlabs::lib::net::rtsp::client::media_type_t::audio)
		{
			add_data(_buffer, frame_size, presentation_time, duration_msec);
		}
	}
    continuePlaying();
}