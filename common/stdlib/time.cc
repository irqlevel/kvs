#include "time.h"
#include <sys/time.h>

namespace Stdlib
{

Clock::Clock()
{
}

Clock::~Clock()
{
}

Time Clock::Now()
{
    struct timeval tv;
    
    BUG_ON(::gettimeofday(&tv, nullptr) != 0);

    return Time(tv.tv_sec, tv.tv_usec);
}

}
