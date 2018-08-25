#ifndef _TIME_HELPER_H_
#define _TIME_HELPER_H_
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>

class TimeHelper 
{
    public:
        static inline uint64_t nowTime() 
        {
            struct timeval t;
            gettimeofday(&t, NULL);
            return (static_cast<uint64_t>(t.tv_sec) * 1000000ul + 
                    static_cast<uint64_t>(t.tv_usec));
        }
};

#endif
