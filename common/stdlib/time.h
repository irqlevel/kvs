#pragma once

#include <common/base.h>

namespace Stdlib
{
    struct Time
    {
        Time(ulong seconds, ulong microseconds)
            : Seconds(seconds)
            , Microseconds(microseconds)
        {
        }
        virtual ~Time() {}
        
        ulong Seconds;
        ulong Microseconds;
    };

    class Clock
    {
    public:
        Clock();
        virtual ~Clock();
        Time Now();
    };
}