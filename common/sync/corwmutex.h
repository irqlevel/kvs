#pragma once

#include <common/base.h>
#include <common/stdlib/result.h>
#include <common/stdlib/list_entry.h>

#include "atomic.h"
#include "spinlock.h"
#include "coroutine.h"
#include "lockable.h"

namespace Sync
{
    class CoRwMutex : public RwLockable
    {
    public:
        CoRwMutex();
        virtual ~CoRwMutex();
        virtual void Lock() override;
        virtual void Unlock() override;
        virtual void ReadLock() override;

        ulong GetMaxSharedWaitersCount();
        ulong GetMaxExclusiveWaitersCount();

    private:
        CoRwMutex(const CoRwMutex& other) = delete;
        CoRwMutex(CoRwMutex&& other) = delete;
        CoRwMutex& operator=(const CoRwMutex& other) = delete;
        CoRwMutex& operator=(CoRwMutex&& other) = delete;

        void WakeupWaiters(bool sharedWaiters);

        Stdlib::ListEntry exclusive_waiters_;
        Stdlib::ListEntry shared_waiters_;

        SpinLock mu_lock_;
        CoroutinePtr owner_;
        ulong shared_owner_count_;

        ulong shared_waiters_count_;
        ulong exclusive_waiters_count_;

        ulong max_shared_waiters_count_;
        ulong max_exclusive_waiters_count_;
    };
}