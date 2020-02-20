#ifndef _ESM_RTC_CLIENT_CORE_H_
#define _ESM_RTC_CLIENT_CORE_H_

#include "esm_rtc_client.h"

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <rtc_base/physical_socket_server.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/thread.h>
#include <system_wrappers/include/field_trial.h>

#include <picojson.h>

// #include <intrin.h>

#include <tbb/concurrent_queue.h>

namespace esmlabs
{
    namespace lib
    {
        namespace net
        {
            namespace rtc
            {
                class client::core
                {
                public:
                    class connection
                    {
                    public:
                        connection(esmlabs::lib::net::rtc::client::core * front)
                            : _front(front)
                            , _pco(*this)
                            , _dco(*this)
                            , _csdo(new ::rtc::RefCountedObject<CSDO>(*this))
                            , _ssdo(new ::rtc::RefCountedObject<SSDO>(*this))
                        {}

                        void onSuccessSessionDescription(webrtc::SessionDescriptionInterface * desc)
                        {
                            _peer_connection->SetLocalDescription(_ssdo, desc);
                            std::string sdp;
                            desc->ToString(&sdp);
                            std::cout << _sdp_type << " SDP:begin" << std::endl << sdp << _sdp_type << " SDP:end" << std::endl;
                        }

                        void onIceCandidate(const webrtc::IceCandidateInterface * candidate)
                        {
                            picojson::object ice;
                            std::string candidate_str;
                            candidate->ToString(&candidate_str);
                            ice.insert(std::make_pair("candidate", picojson::value(candidate_str)));
                            ice.insert(std::make_pair("sdpMid", picojson::value(candidate->sdp_mid())));
                            ice.insert(std::make_pair("sdpMLineIndex", picojson::value(static_cast<double>(candidate->sdp_mline_index()))));
                            _ices.push_back(picojson::value(ice));
                        }

                        class PCO : public webrtc::PeerConnectionObserver
                        {
                        public:
                            PCO(esmlabs::lib::net::rtc::client::core::connection & parent)
                                : _parent(parent)
                            {}

                            void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::SignalingChange(" << new_state << ")" << std::endl;
                            }

                            void OnAddStream(::rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::AddStream" << std::endl;
                            }

                            void OnRemoveStream(::rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::RemoveStream" << std::endl;
                            }

                            void OnDataChannel(::rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::DataChannel(" << data_channel << ", " << _parent._data_channel.get() << ")" << std::endl;

                                _parent._data_channel = data_channel;
                                _parent._data_channel->RegisterObserver(&_parent._dco);
                            }

                            void OnRenegotiationNeeded(void) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::RenegotiationNeeded" << std::endl;
                            }

                            void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::IceConnectionChange(" << new_state << ")" << std::endl;
                            }

                            void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::IceGatheringChange(" << new_state << ")" << std::endl;
                            }

                            void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "PeerConnectionObserver::IceCandidate" << std::endl;
                                _parent.onIceCandidate(candidate);
                            }

                        private:
                            esmlabs::lib::net::rtc::client::core::connection & _parent;
                        };

                        class DCO : public webrtc::DataChannelObserver
                        {
                        public:
                            DCO(esmlabs::lib::net::rtc::client::core::connection & parent)
                                : _parent(parent)
                            {}

                            void OnStateChange(void) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "DataChannelObserver::StateChange" << std::endl;
                            }

                            void OnMessage(const webrtc::DataBuffer& buffer) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "DataChannelObserver::Message" << std::endl;
                                std::cout << std::string(buffer.data.data<char>(), buffer.data.size()) << std::endl;
                                _parent._front->on_receive(buffer.data.data<char>());
                            }

                            void OnBufferedAmountChange(uint64_t previous_amount) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "DataChannelObserver::BufferedAmountChange(" << previous_amount << ")" << std::endl;
                            }

                        private:
                            esmlabs::lib::net::rtc::client::core::connection & _parent;
                        };

                        class CSDO
                            : public webrtc::CreateSessionDescriptionObserver
                        {
                        public:
                            CSDO(esmlabs::lib::net::rtc::client::core::connection & parent)
                                : _parent(parent)
                            {

                            }

                            void OnSuccess(webrtc::SessionDescriptionInterface* desc) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "CreateSessionDescriptionObserver::OnSuccess" << std::endl;
                                _parent.onSuccessSessionDescription(desc);
                            }

                            void OnFailure(const std::string& error) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "CreateSessionDescriptionObserver::OnFailure" << std::endl << error << std::endl;
                            }

                        private:
                            esmlabs::lib::net::rtc::client::core::connection & _parent;
                        };

                        class SSDO
                            : public webrtc::SetSessionDescriptionObserver
                        {
                        public:
                            SSDO(esmlabs::lib::net::rtc::client::core::connection & parent)
                                : _parent(parent)
                            {

                            }

                            void OnSuccess(void) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "SetSessionDescriptionObserver::OnSuccess" << std::endl;
                            }

                            void OnFailure(const std::string& error) override
                            {
                                std::cout << std::this_thread::get_id() << ":" << "SetSessionDescriptionObserver::OnFailure" << std::endl << error << std::endl;
                            }

                        private:
                            esmlabs::lib::net::rtc::client::core::connection & _parent;
                        };

                    public:
                        ::rtc::scoped_refptr<webrtc::PeerConnectionInterface>	_peer_connection;
                        ::rtc::scoped_refptr<webrtc::DataChannelInterface>		_data_channel;
                        std::string												_sdp_type;
                        picojson::array											_ices;

                    private:
                        esmlabs::lib::net::rtc::client::core::connection::PCO _pco;
                        esmlabs::lib::net::rtc::client::core::connection::DCO _dco;
                        ::rtc::scoped_refptr<CSDO> _csdo;
                        ::rtc::scoped_refptr<SSDO> _ssdo;   
                        esmlabs::lib::net::rtc::client::core * _front;                 
                    };

                    explicit core(esmlabs::lib::net::rtc::client * front);
                    ~core(void);

                    int32_t start(int32_t portnumber);
                    int32_t stop(void);

                    int32_t send(std::string & message);
                    void    on_receive(std::string message);

                private:
                    static void *  process_cb(void * param);
                    void    process(void);

                private:
                    esmlabs::lib::net::rtc::client * _front;
                    bool            _run;
                    int32_t         _tid;
                    pthread_t       _thread;
                    pthread_mutex_t _lock;
                    pthread_cond_t  _cond;

                    int32_t         _portnumber;

                    tbb::concurrent_queue<std::string> _queue;
                };
            };
        };
    };
};


#endif