#include "waitgroup.h"
#include <assert.h>

namespace Sync
{

WaitGroup::WaitGroup()
{
    counter_.Set(0);
}

WaitGroup::~WaitGroup()
{
}

void WaitGroup::Add(long delta)
{
    assert(delta > 0);
    auto value = counter_.Add(delta);
    (void)value;
    assert(value >= delta);
}

bool WaitGroup::IsDone()
{
    return (counter_.Get() == 0);
}

bool WaitGroup::IsDone(void *ctx)
{
    WaitGroup* wg = static_cast<WaitGroup*>(ctx);
    return wg->IsDone();
}

void WaitGroup::Done()
{
    auto value = counter_.Dec();
    assert(value >= 0);
    if (value == 0) {
        //signal all threads blocked inside Wait()
        cond_wait_.Broadcast(lock_);
    }
}

void WaitGroup::Wait()
{
    cond_wait_.Wait(lock_, &WaitGroup::IsDone, this);
}

}