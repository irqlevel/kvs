#include "condwait.h"
#include <assert.h>

namespace Sync
{

CondWait::CondWait()
{
    assert(pthread_cond_init(&cond_wait_, NULL) == 0);
}

CondWait::~CondWait()
{
    assert(pthread_cond_destroy(&cond_wait_) == 0);
}

void CondWait::Wait(Mutex& lock, CondFunction func, void* context)
{
    lock.Lock();
    while (!func(context)) { 
        assert(pthread_cond_wait(&cond_wait_, lock.GetRawLock()) == 0);
    }
    lock.Unlock();
}

void CondWait::Signal(Mutex& lock)
{
    lock.Lock();
    assert(pthread_cond_signal(&cond_wait_) == 0);
    lock.Unlock();
}

void CondWait::Broadcast(Mutex& lock)
{
    lock.Lock();
    assert(pthread_cond_broadcast(&cond_wait_) == 0);
    lock.Unlock();
}

}