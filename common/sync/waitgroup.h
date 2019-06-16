#pragma once

#include <common/base.h>
#include "atomic.h"
#include "condwait.h"
#include "mutex.h"

namespace Sync
{

class WaitGroup
{
public:
    WaitGroup();
    virtual ~WaitGroup();

    void Add(long delta);
    void Done();
    void Wait();
    bool IsDone();

private:
    WaitGroup(const WaitGroup& other) = delete;
    WaitGroup(WaitGroup&& other) = delete;
    WaitGroup& operator=(const WaitGroup& other) = delete;
    WaitGroup& operator=(WaitGroup&& other) = delete;

    static bool IsDone(void *ctx);

    Atomic counter_;
    Mutex lock_;
    CondWait cond_wait_;
};

}