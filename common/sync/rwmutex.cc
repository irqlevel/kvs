#include "rwmutex.h"
#include <assert.h>

namespace Sync
{

RWMutex::RWMutex()
{
    assert(pthread_rwlock_init(&lock_, NULL) == 0);
}

RWMutex::~RWMutex()
{
    assert(pthread_rwlock_destroy(&lock_) == 0);
}

void RWMutex::Lock()
{
    assert(pthread_rwlock_wrlock(&lock_) == 0);
}

void RWMutex::Unlock()
{
    assert(pthread_rwlock_unlock(&lock_) == 0);
}

void RWMutex::ReadLock()
{
    assert(pthread_rwlock_rdlock(&lock_) == 0);
}

}