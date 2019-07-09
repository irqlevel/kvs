#include "condwait.h"

namespace Sync
{

CondWait::CondWait()
{
    auto ret = pthread_cond_init(&cond_wait_, NULL);
    if (ret)
        BUG_ON(1);
}

CondWait::~CondWait()
{
    auto ret = pthread_cond_destroy(&cond_wait_);
    if (ret)
        BUG_ON(1);
}

void CondWait::Wait(Mutex& lock, CondFunction func, void* context)
{
    lock.Lock();
    while (!func(context)) { 
        auto ret = pthread_cond_wait(&cond_wait_, lock.GetRawLock());
        if (ret)
            BUG_ON(1);
    }
    lock.Unlock();
}

void CondWait::Signal(Mutex& lock)
{
    lock.Lock();
    auto ret = pthread_cond_signal(&cond_wait_);
    if (ret)
        BUG_ON(1);
    lock.Unlock();
}

void CondWait::Broadcast(Mutex& lock)
{
    lock.Lock();
    auto ret = pthread_cond_broadcast(&cond_wait_);
    if (ret)
        BUG_ON(1);
    lock.Unlock();
}

}