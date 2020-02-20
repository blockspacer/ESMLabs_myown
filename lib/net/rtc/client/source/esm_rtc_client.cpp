#include "esm_rtc_client.h"
#include "rtc_client.h"

esmlabs::lib::net::rtc::client::client(void)
    : _core(nullptr)
{
    _core = new esmlabs::lib::net::rtc::client::core(this);
}

esmlabs::lib::net::rtc::client::~client(void)
{
    if(_core)
    {
        delete _core;
        _core = nullptr;
    }
}

int32_t esmlabs::lib::net::rtc::client::start(int32_t portnumber)
{
    return _core->start(portnumber);
}

int32_t esmlabs::lib::net::rtc::client::stop(void)
{
    return _core->stop();
}

int32_t esmlabs::lib::net::rtc::client::send(std::string & message)
{
    return _core->send(message);
}