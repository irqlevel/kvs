#include "mutex.h"

namespace Sync
{

Mutex::Mutex()
{
    auto ret = pthread_mutex_init(&lock_, NULL);
    if (ret)
        BUG_ON(1);
}

Mutex::~Mutex()
{
    auto ret = pthread_mutex_destroy(&lock_);
    if (ret)
        BUG_ON(1);
}

void Mutex::Lock()
{
    auto ret = pthread_mutex_lock(&lock_);
    if (ret)
        BUG_ON(1);
}

void Mutex::Unlock()
{
    auto ret = pthread_mutex_unlock(&lock_);
    if (ret)
        BUG_ON(1);
}

pthread_mutex_t* Mutex::GetRawLock()
{
    return &lock_;
}

}