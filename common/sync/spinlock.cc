#include "spinlock.h"

namespace Sync
{

SpinLock::SpinLock()
{
    auto ret = pthread_spin_init(&lock_, PTHREAD_PROCESS_PRIVATE);
    if (ret)
        BUG_ON(1);
}

SpinLock::~SpinLock()
{
    auto ret = pthread_spin_destroy(&lock_);
    if (ret)
        BUG_ON(1);
}

void SpinLock::Lock()
{
    auto ret = pthread_spin_lock(&lock_);
    if (ret)
        BUG_ON(1);
}

void SpinLock::Unlock()
{
    auto ret = pthread_spin_unlock(&lock_);
    if (ret)
        BUG_ON(1);
}

pthread_spinlock_t* SpinLock::GetRawLock()
{
    return &lock_;
}

}