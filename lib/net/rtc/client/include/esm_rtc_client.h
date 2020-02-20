#ifndef _ESM_RTC_CLIENT_H_
#define _ESM_RTC_CLIENT_H_

#if defined(ESMRTCClient_EXPORTS)
#define EXP_ESM_RTC_CLIENT_CLASS __attribute__((__visibility__("default")))
#else
#define EXP_ESM_RTC_CLIENT_CLASS
#endif

#include <esm.h>

namespace esmlabs
{
    namespace lib
    {
        namespace net
        {
            namespace rtc
            {
                class EXP_ESM_RTC_CLIENT_CLASS client
                    : public esmlabs::base
                {
                    class core;
                public:
                    explicit client(void);
                    virtual ~client(void);

                    int32_t start(int32_t portnumber);
                    int32_t stop(void);
                    int32_t send(std::string & message);
                    virtual void on_receive(std::string message) = 0;

                private:
                    esmlabs::lib::net::rtc::client::core * _core;
                };
            };
        };
    };
};

#endif