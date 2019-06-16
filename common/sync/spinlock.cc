#include "spinlock.h"
#include <assert.h>

namespace Sync
{

SpinLock::SpinLock()
{
    assert(pthread_spin_init(&lock_, PTHREAD_PROCESS_PRIVATE) == 0);
}

SpinLock::~SpinLock()
{
    assert(pthread_spin_destroy(&lock_) == 0);
}

void SpinLock::Lock()
{
    assert(pthread_spin_lock(&lock_) == 0);
}

void SpinLock::Unlock()
{
    assert(pthread_spin_unlock(&lock_) == 0);
}

pthread_spinlock_t* SpinLock::GetRawLock()
{
    return &lock_;
}

}