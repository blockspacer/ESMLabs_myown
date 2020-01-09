#ifndef _ESMLABS_LOCKS_H_
#define _ESMLABS_LOCKS_H_

#include <pthread.h>

namespace esmlabs
{
    namespace locks
    {
        class autolock
        {
        public:
            autolock(pthread_mutex_t * lock)
                : _lock(lock)
            {
                ::pthread_mutex_lock(_lock);
            }

            ~autolock(void)
            {
                ::pthread_mutex_unlock(_lock);    
            }

        private:
            pthread_mutex_t * _lock;
        };
    };
};


#endif