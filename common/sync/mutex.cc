#include "mutex.h"
#include <assert.h>

namespace Sync
{

Mutex::Mutex()
{
    assert(pthread_mutex_init(&lock_, NULL) == 0);
}

Mutex::~Mutex()
{
    assert(pthread_mutex_destroy(&lock_) == 0);
}

void Mutex::Lock()
{
    assert(pthread_mutex_lock(&lock_) == 0);
}

void Mutex::Unlock()
{
    assert(pthread_mutex_unlock(&lock_) == 0);
}

pthread_mutex_t* Mutex::GetRawLock()
{
    return &lock_;
}

}