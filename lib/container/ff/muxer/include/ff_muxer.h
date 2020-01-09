#ifndef _FF_MUXER_V4_2_1_H_
#define _FF_MUXER_V4_2_1_H_

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavfilter/avfilter.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/opt.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}
#include <tbb/concurrent_queue.h>
#include <chrono>
#include <unistd.h>
#include <atomic>
#include <esm_locks.h>
#include "esm_ff_muxer.h"

namespace esmlabs
{
    namespace lib
    {
        namespace container
        {
            namespace ff
            {
                class muxer::core
                {
                public:
                    typedef struct _queue_elem_t
                    {
                        int32_t		type;
                        uint8_t *	data;
                        int32_t		size;
                        long long   dts;
                        long long   cts;
                        long long   timestamp;
                        _queue_elem_t(int size)
                            : type(esmlabs::lib::container::ff::muxer::media_type_t::unknown)
                            , data(nullptr)
                            , size(size)
                            , dts(-1)
                            , cts(-1)
                        {
                            data = (unsigned char*)malloc(size);
                            ::memset(data, 0x00, size);
                        }
                        ~_queue_elem_t(void)
                        {
                            if (data)
                            {
                                free(data);
                                data = nullptr;
                            }
                            size = 0;
                        }
                        _queue_elem_t(const _queue_elem_t & clone)
                        {
                            type = clone.type;
                            data = clone.data;
                            size = clone.size;
                            dts = clone.dts;
                            cts = clone.cts;
                        }
                        _queue_elem_t & operator=(const _queue_elem_t & clone)
                        {
                            type = clone.type;
                            data = clone.data;
                            size = clone.size;
                            dts = clone.dts;
                            cts = clone.cts;
                            return (*this);
                        }
                    } queue_elem_t;
                
                    typedef struct _cmd_type_t
                    {
                        static const int put_video = 0;
                        static const int put_audio = 1;
                    } cmd_type_t;

                    typedef struct _msg_t
                    {
                        int32_t		msg_type;
                        char		path[MAX_PATH];
                        std::shared_ptr<esmlabs::lib::container::ff::muxer::core::queue_elem_t> msg_elem;
                    } msg_t;

                    typedef struct _media_thread_context_t
                    {
                        char		path[MAX_PATH];
                        esmlabs::lib::container::ff::muxer::core * parent;
                    } media_thread_context_t;

                    core(esmlabs::lib::container::ff::muxer * front);
                    ~core(void);                    

                    bool	is_initialized(void);
                    int32_t initialize(esmlabs::lib::container::ff::muxer::context_t * context);
                    int32_t release(void);
                    int32_t put_video_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts);
                    int32_t put_audio_stream(uint8_t * bytes, int32_t nbytes, long long dts, long long cts);

                private:
                    void	fill_video_stream(AVStream * vs, int32_t id);
                    void	fill_audio_stream(AVStream * as, int32_t id);

                    static void *   process_cb(void * param);
                    static void *   process_media_cb(void * param);
                    void	        process(void);
                    void	        process_media(const char * path);

                private:
                    esmlabs::lib::container::ff::muxer *				_front;
                    esmlabs::lib::container::ff::muxer::context_t *	_context;

                    bool	    _is_initialized;
                    bool	    _run;
                    bool	    _media_run;

                    pthread_t   _thread;
                    pthread_t	_media_thread;

                    std::atomic<long long>    _frame_number;
                    std::atomic<long long>    _section_number;
                    std::atomic<long long>    _video_count;
                    std::atomic<long long>    _audio_count;
                    long long   _start_time;
                    bool		_delayed_muxer_init;

                    tbb::concurrent_queue<std::shared_ptr<muxer::core::msg_t>>			_msg_queue;
                    tbb::concurrent_queue<std::shared_ptr<muxer::core::queue_elem_t>>	_media_queue;

                    pthread_mutex_t	_lock;

                };
            };
        };
    };
};












#endif