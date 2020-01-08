#include "ff_muxer_v4.2.1.h"

magnetar::lib::container::ff::v4_2_1::muxer::core::core(magnetar::lib::container::ff::v4_2_1::muxer * front)
	: _front(front)
	, _context(nullptr)
	, _is_initialized(false)
	, _thread()
	, _run(false)
	, _media_thread()
	, _media_run(false)
	, _frame_number(0)
	, _section_number(0)
	, _video_count(0)
	, _audio_count(0)
{
	::pthread_mutex_init(&_lock, nullptr);
}

magnetar::lib::container::ff::v4_2_1::muxer::core::~core(void)
{
	::pthread_mutex_destroy(&_lock);
}

bool magnetar::lib::container::ff::v4_2_1::muxer::core::is_initialized(void)
{
	return _is_initialized;
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::core::initialize(magnetar::lib::container::ff::v4_2_1::muxer::context_t * context)
{
	if (!(context->option & media_type_t::video) && !(context->option & media_type_t::audio))
		return magnetar::lib::container::ff::v4_2_1::muxer::err_code_t::generic_fail;

    magnetar::locks::autolock lock(&_lock);

	_context = context;
	_frame_number = 0;
	_section_number = 0;
	_video_count = 0;
	_audio_count = 0;
	_start_time = -1;
	_delayed_muxer_init = false;
	_run = true;
    ::pthread_create(&_thread, nullptr, magnetar::lib::container::ff::v4_2_1::muxer::core::process_cb, (void*)this);
	_is_initialized = true;
	return magnetar::lib::container::ff::v4_2_1::muxer::err_code_t::success;
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::core::release(void)
{
    magnetar::locks::autolock lock(&_lock);

	_is_initialized = false;
	_run = false;
    ::pthread_join(_thread, nullptr);
	_msg_queue.clear();
	return magnetar::lib::container::ff::v4_2_1::muxer::err_code_t::success;
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::core::put_video_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts)
{
	if (!bytes || nbytes < 1)
		return magnetar::lib::container::ff::v4_2_1::muxer::err_code_t::success;

    magnetar::locks::autolock lock(&_lock);        

	std::shared_ptr<muxer::core::msg_t> msgQueueElem = std::shared_ptr<muxer::core::msg_t>(new muxer::core::msg_t);
	msgQueueElem->msg_type = muxer::core::cmd_type_t::put_video;
	
	std::shared_ptr<muxer::core::queue_elem_t> videoQueueElem = std::shared_ptr<muxer::core::queue_elem_t>(new muxer::core::queue_elem_t(nbytes));
	videoQueueElem->type = magnetar::lib::container::ff::v4_2_1::muxer::media_type_t::video;
	::memmove(videoQueueElem->data, bytes, nbytes);
	videoQueueElem->size = nbytes;
	videoQueueElem->dts = dts;
	videoQueueElem->cts = cts;

    auto now = std::chrono::system_clock::now();
    auto now_microsec = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto value = now_microsec.time_since_epoch();
	videoQueueElem->timestamp = value.count();
	if (_start_time == -1)
		_start_time = videoQueueElem->timestamp;

	msgQueueElem->msg_elem = videoQueueElem;
	_msg_queue.push(msgQueueElem);

	return muxer::err_code_t::success;
}

int32_t magnetar::lib::container::ff::v4_2_1::muxer::core::put_audio_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts)
{
	if (!bytes || nbytes < 1)
		return magnetar::lib::container::ff::v4_2_1::muxer::err_code_t::success;

    magnetar::locks::autolock lock(&_lock);    

	std::shared_ptr<muxer::core::msg_t> msgQueueElem = std::shared_ptr<muxer::core::msg_t>(new muxer::core::msg_t);
	msgQueueElem->msg_type = muxer::core::cmd_type_t::put_audio;

	std::shared_ptr<muxer::core::queue_elem_t> audioQueueElem = std::shared_ptr<muxer::core::queue_elem_t>(new muxer::core::queue_elem_t(nbytes));
	audioQueueElem->type = magnetar::lib::container::ff::v4_2_1::muxer::media_type_t::audio;
	::memmove(audioQueueElem->data, bytes, nbytes);
	audioQueueElem->size = nbytes;
	audioQueueElem->dts = dts;
	audioQueueElem->cts = cts;

    auto now = std::chrono::system_clock::now();
    auto now_microsec = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto value = now_microsec.time_since_epoch();
	audioQueueElem->timestamp = value.count();
	if (_start_time == -1)
		_start_time = audioQueueElem->timestamp;

	msgQueueElem->msg_elem = audioQueueElem;
	_msg_queue.push(msgQueueElem);
	return muxer::err_code_t::success;
}

void magnetar::lib::container::ff::v4_2_1::muxer::core::process(void)
{
	int64_t start = -1;
	std::shared_ptr<muxer::core::msg_t> msgQueueElem;
	while (_run)
	{
		bool result = _msg_queue.try_pop(msgQueueElem);
		if (result)
		{
			switch (msgQueueElem->msg_type)
			{
			case muxer::core::cmd_type_t::put_video:
			{
				std::shared_ptr<muxer::core::queue_elem_t> videoQueueElem = msgQueueElem->msg_elem;

				if (_context->generation_rule == magnetar::lib::container::ff::v4_2_1::muxer::generation_rule_t::full)
				{
					if (_frame_number == 0)
					{
						core::media_thread_context_t * tctx = new core::media_thread_context_t();
						tctx->parent = this;
						::memset(tctx->path, 0x00, sizeof(tctx->path));
						::snprintf(tctx->path, sizeof(tctx->path), "%s_%lld.mp4", _context->path, _section_number.load(std::memory_order_relaxed));

						_media_run = true;
                        ::pthread_create(&_media_thread, nullptr, muxer::core::process_media_cb, tctx);
					}
				}
				else if (_context->generation_rule == magnetar::lib::container::ff::v4_2_1::muxer::generation_rule_t::frame)
				{
					if ((_frame_number % (_context->generation_threshold - 1)) == 0)
					{
						if (_frame_number != 0)
						{
							_media_run = false;
                            ::pthread_join(_media_thread, nullptr);
                            _section_number.fetch_add(1, std::memory_order_relaxed);
						}

						core::media_thread_context_t * tctx = new core::media_thread_context_t();
						tctx->parent = this;
						::memset(tctx->path, 0x00, sizeof(tctx->path));
						::snprintf(tctx->path, sizeof(tctx->path), "%s_%lld.mp4", _context->path, _section_number.load(std::memory_order_relaxed));

						_media_run = true;
                        ::pthread_create(&_media_thread, nullptr, muxer::core::process_media_cb, tctx);
					}
				}
				else if (_context->generation_rule == magnetar::lib::container::ff::v4_2_1::muxer::generation_rule_t::time)
				{
					if (_frame_number == 0)
					{
						start = videoQueueElem->timestamp;

						core::media_thread_context_t * tctx = new core::media_thread_context_t();
						tctx->parent = this;

						::memset(tctx->path, 0x00, sizeof(tctx->path));
						::snprintf(tctx->path, sizeof(tctx->path), "%s_%lld.mp4", _context->path, _section_number.load(std::memory_order_relaxed));

						_media_run = true;
                        ::pthread_create(&_media_thread, nullptr, muxer::core::process_media_cb, tctx);                        
					}
					else
					{
						long long diff = videoQueueElem->timestamp - start;
						if (diff > (_context->generation_threshold * 1000))
						{
							_media_run = false;
                            ::pthread_join(_media_thread, nullptr); 
                            _section_number.fetch_add(1, std::memory_order_relaxed);
							start = videoQueueElem->timestamp;

							core::media_thread_context_t * tctx = new core::media_thread_context_t();
							tctx->parent = this;
							::memset(tctx->path, 0x00, sizeof(tctx->path));
							::snprintf(tctx->path, sizeof(tctx->path), "%s_%lld.mp4", _context->path, _section_number.load(std::memory_order_relaxed));

							_media_run = true;
                            ::pthread_create(&_media_thread, nullptr, muxer::core::process_media_cb, tctx);
						}
					}
				}

				_media_queue.push(videoQueueElem);
                _frame_number.fetch_add(1, std::memory_order_relaxed);
				break;
			}

			case muxer::core::cmd_type_t::put_audio:
			{
				std::shared_ptr<muxer::core::queue_elem_t> audioQueueElem = msgQueueElem->msg_elem;
				_media_queue.push(audioQueueElem);
				break;
			}
			}
		}
		else
		{
            ::usleep(5000);
		}
	}

	// while (!_msg_queue.empty())
	// 	_msg_queue.try_pop(msgQueueElem);

	_media_run = false;
    ::pthread_join(_media_thread, nullptr);
	_media_queue.clear();
}

void magnetar::lib::container::ff::v4_2_1::muxer::core::process_media(const char * path)
{
	AVFormatContext	*	format_ctx = nullptr;
	AVOutputFormat *	format = nullptr;
	AVStream *			video_stream = nullptr;
	AVStream *			audio_stream = nullptr;

	int32_t *	stream_mapping = nullptr;
	int32_t		stream_mapping_size = 0;
	int32_t		stream_index = 0;
	::avformat_alloc_output_context2(&format_ctx, nullptr, nullptr, path);
	if (!format_ctx)
		return;

	if ((_context->option & media_type_t::video) && (_context->option & media_type_t::audio))
		stream_mapping_size = 2;
	else
		stream_mapping_size = 1;
	stream_mapping = static_cast<int32_t*>(::av_mallocz_array(2, sizeof(*stream_mapping)));

	format = format_ctx->oformat;
	for (int32_t i = 0; i < stream_mapping_size; i++)
	{
		stream_mapping[i] = stream_index++;
		if ((_context->option & media_type_t::video) && (_context->option & media_type_t::audio))
		{
			if (i == 0)
			{
				video_stream = ::avformat_new_stream(format_ctx, nullptr);
				fill_video_stream(video_stream, i);
			}
			else
			{
				audio_stream = ::avformat_new_stream(format_ctx, nullptr);
				fill_audio_stream(audio_stream, i);
			}
		}
		else if (_context->option & media_type_t::video)
		{
			video_stream = ::avformat_new_stream(format_ctx, nullptr);
			fill_video_stream(video_stream, i);
		}
		else
		{
			audio_stream = ::avformat_new_stream(format_ctx, nullptr);
			fill_audio_stream(audio_stream, i);
		}
	}
	::av_dump_format(format_ctx, 0, path, 1);
	int32_t ret = ::avio_open(&format_ctx->pb, path, AVIO_FLAG_WRITE);
	ret = ::avformat_write_header(format_ctx, nullptr);

	std::shared_ptr<muxer::core::queue_elem_t> queueElem;
	AVRational	bq;

	uint8_t *	vbytes = nullptr;
	int32_t		nvbytes = 0;
	long long 	vpts = 0;
	long long 	vdts = 0;
	long long 	vduration = 0;

	uint8_t *	abytes = nullptr;
	int32_t		nabytes = 0;
	long long 	apts = 0;
	long long 	adts = 0;
	long long 	aduration = 0;

	AVPacket	pkt = { 0 };

	while (_media_run)
	{
		while (_media_queue.try_pop(queueElem))
		{
			if (queueElem->type == magnetar::lib::container::ff::v4_2_1::muxer::media_type_t::video)
			{
				vbytes = queueElem->data;
				nvbytes = queueElem->size;
				if (_context->timestamp_mode == magnetar::lib::container::ff::v4_2_1::muxer::timestamp_mode_t::frame_count)
				{
					bq.num = _context->video_fps_num;
					bq.den = _context->video_fps_den;
					vpts = ::av_rescale_q(_video_count, bq, video_stream->time_base);
					vduration = ::av_rescale_q(1, bq, video_stream->time_base);
					_video_count++;
				}
				else if(_context->timestamp_mode == magnetar::lib::container::ff::v4_2_1::muxer::timestamp_mode_t::recved_time)
				{
					bq.num = 1;
					bq.den = 1000000;
					vpts = ::av_rescale_q(queueElem->timestamp - _start_time, bq, video_stream->time_base);
					vdts = vpts;
					vduration = ::av_rescale_q(1, bq, video_stream->time_base);
				}
				else if (_context->timestamp_mode == magnetar::lib::container::ff::v4_2_1::muxer::timestamp_mode_t::passed_time)
				{
					vpts = ::av_rescale_q(queueElem->dts + queueElem->cts, AVRational{ _context->video_tb_num, _context->video_tb_den }, video_stream->time_base);
					vdts = ::av_rescale_q(queueElem->dts, AVRational{ _context->video_tb_num, _context->video_tb_den }, video_stream->time_base);
					vduration = ::av_rescale_q(1, AVRational{ _context->video_tb_num, _context->video_tb_den }, video_stream->time_base);
				}

				::av_init_packet(&pkt);
				pkt.pts = vpts;
				pkt.dts = vdts;
				pkt.duration = vduration;
				pkt.pos = -1;
				pkt.stream_index = video_stream->index;
				pkt.data = vbytes;
				pkt.size = nvbytes;
				int32_t ret = ::av_write_frame(format_ctx, &pkt);
				::av_packet_unref(&pkt);

			}
			else if (queueElem->type == magnetar::lib::container::ff::v4_2_1::muxer::media_type_t::audio && (_context->option & magnetar::lib::container::ff::v4_2_1::muxer::media_type_t::audio))
			{
				abytes = queueElem->data;
				nabytes = queueElem->size;

				if (_context->timestamp_mode == magnetar::lib::container::ff::v4_2_1::muxer::timestamp_mode_t::frame_count)
				{
					bq.num = audio_stream->codecpar->frame_size * 10000;
					bq.den = _context->audio_samplerate * 10000;
					apts = ::av_rescale_q(_audio_count, bq, audio_stream->time_base);
					aduration = ::av_rescale_q(1, bq, audio_stream->time_base);
					_audio_count++;
				}
				else if (_context->timestamp_mode == magnetar::lib::container::ff::v4_2_1::muxer::timestamp_mode_t::recved_time)
				{
					bq.num = 1;
					bq.den = 1000000;
					apts = ::av_rescale_q(queueElem->timestamp - _start_time, bq, audio_stream->time_base);
					adts = apts;
					aduration = ::av_rescale_q(1, bq, audio_stream->time_base);
				}
				else if (_context->timestamp_mode == magnetar::lib::container::ff::v4_2_1::muxer::timestamp_mode_t::passed_time)
				{
					apts = ::av_rescale_q(queueElem->dts + queueElem->cts, AVRational{ _context->audio_tb_num, _context->audio_tb_den }, audio_stream->time_base);
					adts = ::av_rescale_q(queueElem->dts, AVRational{ _context->audio_tb_num, _context->audio_tb_den }, audio_stream->time_base);
					aduration = ::av_rescale_q(1, AVRational{ _context->audio_tb_num, _context->audio_tb_den }, audio_stream->time_base);
				}
				
				::av_init_packet(&pkt);
				pkt.pts = apts;
				pkt.dts = adts;
				pkt.duration = aduration;
				pkt.pos = -1;
				pkt.stream_index = audio_stream->index;
				pkt.data = abytes;
				pkt.size = nabytes;
				ret = ::av_write_frame(format_ctx, &pkt);
				::av_packet_unref(&pkt);
			}
		}
	}

	::av_write_trailer(format_ctx);
	::avio_close(format_ctx->pb);
	::avformat_free_context(format_ctx);
	::av_free(stream_mapping);
}

void magnetar::lib::container::ff::v4_2_1::muxer::core::fill_video_stream(AVStream * vs, int32_t id)
{
	vs->id = id;
	vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	switch (_context->video_codec)
	{
	case magnetar::lib::container::ff::v4_2_1::muxer::video_codec_type_t::avc:
		vs->codecpar->codec_id = AV_CODEC_ID_H264;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::video_codec_type_t::hevc:
		vs->codecpar->codec_id = AV_CODEC_ID_HEVC;
		break;
	}
	if (!_context->ignore_video_extradata && _context->video_extradata_size > 0)
	{
		vs->codecpar->extradata_size = _context->video_extradata_size;
		vs->codecpar->extradata = static_cast<uint8_t*>(av_malloc(_context->video_extradata_size));
		::memmove(vs->codecpar->extradata, _context->video_extradata, vs->codecpar->extradata_size);
	}
	vs->codecpar->width = _context->video_width;
	vs->codecpar->height = _context->video_height;
	vs->r_frame_rate.num = _context->video_fps_num;
	vs->r_frame_rate.den = _context->video_fps_den;
	vs->codecpar->codec_tag = 0;
}

void magnetar::lib::container::ff::v4_2_1::muxer::core::fill_audio_stream(AVStream * as, int32_t id)
{
	as->id = id;
	as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	switch (_context->audio_codec)
	{
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_codec_type_t::aac:
		as->codecpar->codec_id = AV_CODEC_ID_AAC;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_codec_type_t::ac3:
		as->codecpar->codec_id = AV_CODEC_ID_AC3;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_codec_type_t::mp3:
		as->codecpar->codec_id = AV_CODEC_ID_MP3;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_codec_type_t::opus:
		as->codecpar->codec_id = AV_CODEC_ID_OPUS;
		break;
	}

	if (!_context->ignore_audio_extradata && (_context->audio_extradata_size > 0))
	{
		as->codecpar->extradata_size = _context->audio_extradata_size;
		as->codecpar->extradata = static_cast<uint8_t*>(av_malloc(_context->audio_extradata_size));
		::memmove(as->codecpar->extradata, _context->audio_extradata, as->codecpar->extradata_size);
	}

	as->codecpar->sample_rate = _context->audio_samplerate;

	switch (_context->audio_sampleformat)
	{
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_u8 :
		as->codecpar->format = AV_SAMPLE_FMT_U8;
		as->codecpar->bits_per_coded_sample = 8;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_s16:
		as->codecpar->format = AV_SAMPLE_FMT_S16;
		as->codecpar->bits_per_coded_sample = 16;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_s32:
		as->codecpar->format = AV_SAMPLE_FMT_S32;
		as->codecpar->bits_per_coded_sample = 32;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_flt:
		as->codecpar->format = AV_SAMPLE_FMT_FLT;
		as->codecpar->bits_per_coded_sample = 32;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_dbl:
		as->codecpar->format = AV_SAMPLE_FMT_DBL;
		as->codecpar->bits_per_coded_sample = 64;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_s64:
		as->codecpar->format = AV_SAMPLE_FMT_S64;
		as->codecpar->bits_per_coded_sample = 64;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_u8p:
		as->codecpar->format = AV_SAMPLE_FMT_U8P;
		as->codecpar->bits_per_coded_sample = 8;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_s16p:
		as->codecpar->format = AV_SAMPLE_FMT_S16P;
		as->codecpar->bits_per_coded_sample = 16;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_s32p:
		as->codecpar->format = AV_SAMPLE_FMT_S32P;
		as->codecpar->bits_per_coded_sample = 32;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_fltp:
		as->codecpar->format = AV_SAMPLE_FMT_FLTP;
		as->codecpar->bits_per_coded_sample = 16;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_dblp:
		as->codecpar->format = AV_SAMPLE_FMT_DBLP;
		as->codecpar->bits_per_coded_sample = 64;
		break;
	case magnetar::lib::container::ff::v4_2_1::muxer::audio_sample_format_t::fmt_s64p:
		as->codecpar->format = AV_SAMPLE_FMT_S64P;
		as->codecpar->bits_per_coded_sample = 64;
		break;
	}
	as->codecpar->channels = _context->audio_channels;
	as->codecpar->frame_size = _context->audio_frame_size;
	as->codecpar->codec_tag = 0;
}

void * magnetar::lib::container::ff::v4_2_1::muxer::core::process_cb(void * param)
{
	muxer::core * self = static_cast<muxer::core*>(param);
	self->process();
	return 0;
}

void * magnetar::lib::container::ff::v4_2_1::muxer::core::process_media_cb(void * param)
{
	muxer::core::media_thread_context_t * tc = static_cast<muxer::core::media_thread_context_t*>(param);
	if (tc && tc->parent)
	{
		tc->parent->process_media(tc->path);
		delete tc;
		tc = nullptr;
	}
	return 0;
}
