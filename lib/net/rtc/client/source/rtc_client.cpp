#include "rtc_client.h"
#include <esm_locks.h>

esmlabs::lib::net::rtc::client::core::core(esmlabs::lib::net::rtc::client * front)
    : _front(front)
    , _run(false)
    , _tid(-1)
    , _portnumber(-1)
{
    ::pthread_mutex_init(&_lock, nullptr);
    ::pthread_cond_init(&_cond, nullptr);
}

esmlabs::lib::net::rtc::client::core::~core(void)
{
    ::pthread_cond_destroy(&_cond);
    ::pthread_mutex_destroy(&_lock);
}

int32_t esmlabs::lib::net::rtc::client::core::start(int32_t portnumber)
{
    _run = true;
    _tid = pthread_create(&_thread, nullptr, esmlabs::lib::net::rtc::client::core::process_cb, (void*)this);
    return esmlabs::lib::net::rtc::client::err_code_t::success;
}

int32_t esmlabs::lib::net::rtc::client::core::stop(void)
{
    _run = false;
    pthread_join(_thread, nullptr);
    _queue.clear();
    return esmlabs::lib::net::rtc::client::err_code_t::success;
}

int32_t esmlabs::lib::net::rtc::client::core::send(std::string & message)
{
    if(!_run)
        return esmlabs::lib::net::rtc::client::err_code_t::generic_fail;
    
    _queue.push(message);

    esmlabs::locks::autolock lock(&_lock);
    ::pthread_cond_signal(&_cond);
    return esmlabs::lib::net::rtc::client::err_code_t::success;
}

void esmlabs::lib::net::rtc::client::core::on_receive(std::string message)
{
    if(_front)
        _front->on_receive(message);
}

void * esmlabs::lib::net::rtc::client::core::process_cb(void * param)
{
    esmlabs::lib::net::rtc::client::core * self = static_cast<esmlabs::lib::net::rtc::client::core*>(param);
    self->process();
    return 0;
}

void  esmlabs::lib::net::rtc::client::core::process(void)
{
    std::unique_ptr<::rtc::Thread>      networkThread;
    std::unique_ptr<::rtc::Thread>      workerThread;
    std::unique_ptr<::rtc::Thread>      signalingThread;
    ::rtc::scoped_refptr<::webrtc::PeerConnectionFactoryInterface> peerConnectionFactory;
    ::webrtc::PeerConnectionInterface::RTCConfiguration configuration;
    esmlabs::lib::net::rtc::client::core::connection conn(this);

    ::webrtc::field_trial::InitFieldTrialsFromString("");
    ::webrtc::PeerConnectionInterface::IceServer iceServer;
    iceServer.uri = "stun:stun.1.google.com:19302";
    configuration.servers.push_back(iceServer);

    networkThread = ::rtc::Thread::CreateWithSocketServer();
    networkThread->Start();
    workerThread = ::rtc::Thread::Create();
    workerThread->Start();
    signalingThread = ::rtc::Thread::Create();
    signalingThread->Start();
    ::webrtc::PeerConnectionFactoryDependencies dependencies;
    dependencies.network_thread = networkThread.get();
    dependencies.worker_thread = workerThread.get();
    dependencies.signaling_thread = signalingThread.get();
    peerConnectionFactory = ::webrtc::CreateModularPeerConnectionFactory(std::move(dependencies));

    if(peerConnectionFactory.get()==nullptr)
        return;

    while(_run)
    {
        esmlabs::locks::autolock lock(&_lock);
        ::pthread_cond_wait(&_cond, &_lock);
        std::string message;
        if(_queue.try_pop(message))
        {
            ::webrtc::DataBuffer buffer(::rtc::CopyOnWriteBuffer(message.c_str(), message.size()), true);
            conn._data_channel->Send(buffer);
        }
    }    
}