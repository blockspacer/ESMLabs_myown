#ifndef _ESM_FF_MUXER_H_
#define _ESM_FF_MUXER_H_

#if defined(ESMFFMuxer_EXPORTS)
#define EXP_ESM_FF_MUXER_CLASS __attribute__((__visibility__("default")))
#else
#define EXP_ESM_FF_MUXER_CLASS
#endif

#include <esm.h>

namespace esmlabs
{
    namespace lib
    {
        namespace container
        {
            namespace ff
            {
                class EXP_ESM_FF_MUXER_CLASS muxer
                    : public esmlabs::base
                {
                public:
                    class core;

                public:
                    typedef struct _timestamp_mode_t
                    {
                        static const int32_t passed_time = 0;
                        static const int32_t recved_time = 1;
                        static const int32_t frame_count = 2;
                    } timestamp_mode_t;

                    typedef struct _generation_rule_t
                    {
                        static const int32_t full = 0;
                        static const int32_t time = 1;
                        static const int32_t frame = 2;
                    } generation_rule_t;

                    typedef struct _context_t
                    {
                        int32_t		option;
                        char		path[MAX_PATH];

                        int32_t		timestamp_mode;
                        int32_t		generation_rule;
                        int32_t		generation_threshold;//size(MB) or time(millisecond)

                        int32_t		video_codec;
                        uint8_t		video_extradata[500];
                        int32_t		video_extradata_size;
                        bool		ignore_video_extradata;
                        int32_t		video_width;
                        int32_t		video_height;
                        int32_t		video_fps_num;
                        int32_t		video_fps_den;
                        int32_t		video_tb_num;
                        int32_t		video_tb_den;

                        int32_t		audio_codec;
                        uint8_t		audio_extradata[500];
                        int32_t		audio_extradata_size;
                        bool		ignore_audio_extradata;
                        int32_t		audio_samplerate;
                        int32_t		audio_sampleformat;
                        int32_t		audio_channels;
                        int32_t		audio_frame_size;
                        int32_t		audio_tb_num;
                        int32_t		audio_tb_den;
                        _context_t(void)
                            : option(esmlabs::lib::container::ff::muxer::media_type_t::video | esmlabs::lib::container::ff::muxer::media_type_t::audio)
                            , timestamp_mode(esmlabs::lib::container::ff::muxer::timestamp_mode_t::recved_time)
                            , generation_rule(esmlabs::lib::container::ff::muxer::generation_rule_t::frame)
                            , generation_threshold(1000)
                            , video_codec(esmlabs::lib::container::ff::muxer::video_codec_type_t::unknown)
                            , video_extradata_size(0)
                            , ignore_video_extradata(false)
                            , video_width(1280)
                            , video_height(720)
                            , video_fps_num(24000)
                            , video_fps_den(1001)
                            , video_tb_num(1)
                            , video_tb_den(1000)
                            , audio_codec(esmlabs::lib::container::ff::muxer::audio_codec_type_t::unknown)
                            , audio_extradata_size(0)
                            , ignore_audio_extradata(false)
                            , audio_samplerate(44100)
                            , audio_sampleformat(esmlabs::lib::container::ff::muxer::audio_sample_format_t::fmt_flt)
                            , audio_channels(2)
                            , audio_frame_size(1024)
                            , audio_tb_num(1)
                            , audio_tb_den(1000)
                        {
                            ::memset(path, 0x00, sizeof(path));
                            ::memset(video_extradata, 0x00, sizeof(video_extradata));
                            ::memset(audio_extradata, 0x00, sizeof(audio_extradata));
                        }
                    } context_t;

                    explicit muxer(void);
                    virtual ~muxer(void);

                    bool    is_initialized(void);
                    int32_t initialize(esmlabs::lib::container::ff::muxer::context_t * context);
                    int32_t release(void);

                    int32_t put_video_stream(uint8_t * bytes, int32_t nbytes, long long dts = 0, long long cts = 0);
                    int32_t put_audio_stream(uint8_t * bytes, int32_t nbytes, long long dts = 0, long long cts = 0);

                private:
                    esmlabs::lib::container::ff::muxer::core * _core;
                };
            };
        };
    };
};

#endif