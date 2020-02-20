#ifndef _ESM_RTSP_CLIENT_H_
#define _ESM_RTSP_CLIENT_H_

#if defined(ESMRTSPClient_EXPORTS)
#define EXP_ESM_RTSP_CLIENT_CLASS __attribute__((__visibility__("default")))
#else
#define EXP_ESM_RTSP_CLIENT_CLASS
#endif

#include <esm.h>

namespace esmlabs
{
	namespace lib
	{
		namespace net
		{
			namespace rtsp
			{
				class EXP_ESM_RTSP_CLIENT_CLASS client
					: public esmlabs::base
				{
				public:
					class core;
					class buffer_sink;
					class h2645_buffer_sink;
					class h265_buffer_sink;
					class h264_buffer_sink;
					class aac_buffer_sink;
				public:
					typedef struct _transport_option_t
					{
						static const int32_t rtp_over_udp = 0;
						static const int32_t rtp_over_tcp = 1;
						static const int32_t rtp_over_http = 2;
					} transport_option_t;

					explicit client(void);
					virtual ~client(void);
					
					int32_t play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, int32_t recv_timeout, float scale = 1.f, bool repeat = true);
					int32_t stop(void);
					int32_t pause(void);

					void		set_vps(uint8_t * vps, int32_t vps_size);
					void		set_sps(uint8_t * sps, int32_t sps_size);
					void		set_pps(uint8_t * pps, int32_t pps_size);
					uint8_t *	get_vps(int32_t & vps_size);
					uint8_t *	get_sps(int32_t & sps_size);
					uint8_t *	get_pps(int32_t & pps_size);

					void		set_audio_extradata(uint8_t * extradata, int32_t size);
					uint8_t *	get_audio_extradata(int32_t & size);
					void		set_audio_channels(int32_t channels);
					int32_t		get_audio_channels(void);
					void		set_audio_samplerate(int32_t samplerate);
					int32_t		get_audio_samplerate(void);


					virtual void on_begin_video(int32_t codec, uint8_t * vps, int32_t vpssize, uint8_t * sps, int32_t spssize, uint8_t * pps, int32_t ppssize, const uint8_t * bytes, int32_t nbytes, long long pts);
					virtual void on_recv_video(int32_t codec, const uint8_t * bytes, int32_t nbytes, long long pts);
					virtual void on_end_video(void);

					virtual void on_begin_audio(int32_t codec, uint8_t * config, int32_t config_size, int32_t samplerate, int32_t channels, const uint8_t * bytes, int32_t nbytes, long long pts);
					virtual void on_recv_audio(int32_t codec, const uint8_t * bytes, int32_t nbytes, long long pts);
					virtual void on_end_audio(void);

					//bool ignore_sdp(void);


					static bool is_vps(uint8_t nalu_type);
					static bool is_sps(int32_t codec, uint8_t nalu_type);
					static bool is_pps(int32_t codec, uint8_t nalu_type);
					static bool is_idr(int32_t codec, uint8_t nalu_type);
					static bool is_vlc(int32_t codec, uint8_t nalu_type);
					static const int32_t	find_nal_unit(uint8_t * bitstream, int32_t size, int * nal_start, int * nal_end);
					static const uint8_t *	find_start_code(const uint8_t * __restrict begin, const uint8_t * end, uint32_t * __restrict state);

				private:
					static void *	process_cb(void * data);
					void 	process(void);

				private:
					char	_url[MAX_PATH];
					char	_username[MAX_PATH];
					char	_password[MAX_PATH];
					int32_t _transport_option;
					int32_t _recv_option;
					int32_t _recv_timeout;
					float	_scale;
					bool  	_repeat;

					esmlabs::lib::net::rtsp::client::core * _live;
					bool 	_kill;
					bool  	_ignore_sdp;

					uint8_t _vps[100];
					uint8_t _sps[100];
					uint8_t _pps[100];
					int32_t _vps_size;
					int32_t _sps_size;
					int32_t _pps_size;

					uint8_t _audio_extradata[100];
					int32_t _audio_extradata_size;
					int32_t _audio_channels;
					int32_t _audio_samplerate;

					int32_t 	_tid;
					pthread_t 	_thread;
				};
			};
		};
	};
};

#endif