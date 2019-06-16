#pragma once

#include <common/base.h>
#include <common/sync/lockable.h>

namespace Sync
{

class AutoLock
{
public:
    AutoLock(Lockable& lock)
        : lock_(&lock)
    {
        lock_->Lock();
    }

    virtual ~AutoLock()
    {
        lock_->Unlock();
    }

private:
    AutoLock(const AutoLock& other) = delete;
    AutoLock(AutoLock&& other) = delete;
    AutoLock& operator=(const AutoLock& other) = delete;
    AutoLock& operator=(AutoLock&& other) = delete;

    Lockable *lock_;
};

class ReadAutoLock
{
public:
    ReadAutoLock(RwLockable& lock)
        : lock_(&lock)
    {
        lock_->ReadLock();
    }

    virtual ~ReadAutoLock()
    {
        lock_->Unlock();
    }

private:
    ReadAutoLock(const ReadAutoLock& other) = delete;
    ReadAutoLock(ReadAutoLock&& other) = delete;
    ReadAutoLock& operator=(const ReadAutoLock& other) = delete;
    ReadAutoLock& operator=(ReadAutoLock&& other) = delete;

    RwLockable *lock_;
};

}
