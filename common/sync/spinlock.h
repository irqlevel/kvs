#pragma once

#include <common/base.h>
#include <pthread.h>

#include <common/sync/lockable.h>

namespace Sync
{

class SpinLock : public Lockable
{
public:
    SpinLock();
    virtual ~SpinLock();
    virtual void Lock() override;
    virtual void Unlock() override;
    pthread_spinlock_t* GetRawLock();

private:
    SpinLock(const SpinLock& other) = delete;
    SpinLock(SpinLock&& other) = delete;
    SpinLock& operator=(const SpinLock& other) = delete;
    SpinLock& operator=(SpinLock&& other) = delete;

    pthread_spinlock_t lock_;    
};

}