#include "rwmutex.h"

namespace Sync
{

RWMutex::RWMutex()
{
    auto ret = pthread_rwlock_init(&lock_, NULL);
    if (ret)
        BUG_ON(1);
}

RWMutex::~RWMutex()
{
    auto ret = pthread_rwlock_destroy(&lock_);
    if (ret)
        BUG_ON(1);
}

void RWMutex::Lock()
{
    auto ret = pthread_rwlock_wrlock(&lock_);
    if (ret)
        BUG_ON(1);
}

void RWMutex::Unlock()
{
    auto ret = pthread_rwlock_unlock(&lock_);
    if (ret)
        BUG_ON(1);
}

void RWMutex::ReadLock()
{
    auto ret = pthread_rwlock_rdlock(&lock_);
    if (ret)
        BUG_ON(1);
}

}