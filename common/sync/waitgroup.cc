#include "waitgroup.h"

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
    BUG_ON(delta <= 0);
    counter_.Add(delta);
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
    BUG_ON(value < 0);
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