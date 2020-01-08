#if !defined(_MAGNETAR_H_)
#define _MAGNETAR_H_

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <memory>
#include <string>
#include <string.h>
#include <pthread.h>

namespace magnetar 
{
    class base
    {
    public:
        static const int32_t MAX_PATH = 260;
        typedef struct _err_code_t
        {
            static const int32_t unknown = -1;
            static const int32_t success = 0;
            static const int32_t generic_fail = 1;
            static const int32_t memory_alloc = 2;
            static const int32_t invalid_argument = 3;
        } err_code_t;

        typedef struct _media_type_t
        {
            static const int32_t unknown = 0x00;
            static const int32_t video = 0x01;
            static const int32_t audio = 0x02;
            static const int32_t subtitle = 0x04;
        } media_type_t;

        typedef struct _video_codec_type_t
        {
            static const int32_t unknown = -1;
            static const int32_t avc = 0;
            static const int32_t hevc = 1;
        } video_codec_type_t;

        typedef struct _video_entropy_coding_t
        {
            static const int32_t unknown = -1;
            static const int32_t cabac = 0;
            static const int32_t cavlc = 1;
        } video_entropy_coding_t;

        typedef struct _video_rate_control_t
        {
            static const int32_t unknown = -1;
            static const int32_t cbr = 0;
            static const int32_t vbr = 1;
            static const int32_t abr = 2;
        } video_rate_control_t;

        typedef struct _video_codec_picture_t
        {
            static const int32_t unknown = -1;
            static const int32_t idr = 0;
            static const int32_t i = 1;
            static const int32_t p = 2;
            static const int32_t b = 3;
        } video_codec_picture_t;

        typedef struct _video_memory_type_t
        {
            static const int32_t unknown = -1;
            static const int32_t host = 0;
            static const int32_t opengl = 1;
            static const int32_t vulkan = 2;
            static const int32_t opencl = 3;
            static const int32_t cuda = 4;
        } video_memory_type_t;

        typedef struct _video_engine_t
        {
            static const int32_t unknown = -1;
            static const int32_t automatic = 0;
            static const int32_t nvenc = 1;
            static const int32_t msdkenc = 2;
            static const int32_t vce = 3;
            static const int32_t nvdec = 4;
            static const int32_t msdkdec = 5;
            static const int32_t uvd = 6;
            static const int32_t ff = 7;
        } video_engine_t;
        
        typedef struct _audio_codec_type_t
        {
            static const int32_t unknown = -1;
            static const int32_t aac = 0;
            static const int32_t ac3 = 1;
            static const int32_t mp3 = 2;
            static const int32_t opus = 3;
        } audio_codec_type_t;

        typedef struct _audio_sample_format_t
        {
            static const int32_t unknown = -1;
            static const int32_t fmt_u8 = 0;
            static const int32_t fmt_s16 = 1;
            static const int32_t fmt_s32 = 2;
            static const int32_t fmt_flt = 3;
            static const int32_t fmt_dbl = 4;
            static const int32_t fmt_s64 = 5;
            static const int32_t fmt_u8p = 6;
            static const int32_t fmt_s16p = 7;
            static const int32_t fmt_s32p = 8;
            static const int32_t fmt_fltp = 9;
            static const int32_t fmt_dblp = 10;
            static const int32_t fmt_s64p = 11;            
        } audio_sample_format_t;
    };
};









#endif