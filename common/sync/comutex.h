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
    class CoMutex : public Lockable
    {
    public:
        CoMutex();
        virtual ~CoMutex();
        virtual void Lock() override;
        virtual void Unlock() override;
    private:
        CoMutex(const CoMutex& other) = delete;
        CoMutex(CoMutex&& other) = delete;
        CoMutex& operator=(const CoMutex& other) = delete;
        CoMutex& operator=(CoMutex&& other) = delete;

        void WakeupWaiters();

        Stdlib::ListEntry waiters_;
        SpinLock mu_lock_;
        CoroutinePtr owner_;
    };
}