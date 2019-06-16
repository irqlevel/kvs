#include "corwmutex.h"
#include "coroutine.h"

namespace Sync
{

CoRwMutex::CoRwMutex()
    : owner_(nullptr)
    , shared_owner_count_(0)
    , shared_waiters_count_(0)
    , exclusive_waiters_count_(0)
    , max_shared_waiters_count_(0)
    , max_exclusive_waiters_count_(0)
{
}

CoRwMutex::~CoRwMutex()
{
    mu_lock_.Lock();
    BUG_ON(owner_ != nullptr || shared_owner_count_ != 0);
    WakeupWaiters(false);
    WakeupWaiters(true);
    mu_lock_.Unlock();
}

void CoRwMutex::WakeupWaiters(bool shared_waiters)
{
    if (shared_waiters) {
        while (!shared_waiters_.IsEmpty()) {
            auto co = CONTAINING_RECORD(shared_waiters_.RemoveHead(), Coroutine, WaitListEntry);
            shared_waiters_count_--;
            co->WaitListEntry.Init();
            co->Signal();
            Sync::Coroutine::Put(co);
        }
    } else {
        while (!exclusive_waiters_.IsEmpty()) {
            auto co = CONTAINING_RECORD(exclusive_waiters_.RemoveHead(), Coroutine, WaitListEntry);
            exclusive_waiters_count_--;
            co->WaitListEntry.Init();
            co->Signal();
            Sync::Coroutine::Put(co);
        }
    }
}

void CoRwMutex::Lock()
{
    BUG_ON(!Coroutine::InCoroutine());

    auto self = Coroutine::Self();

    for (;;) {
        mu_lock_.Lock();
        if (owner_ == nullptr && shared_owner_count_ == 0) {
            Sync::Coroutine::Get(self);
            owner_ = self;
            mu_lock_.Unlock();
            return;
        } else {
            if (self->WaitListEntry.IsEmpty()) {
                Sync::Coroutine::Get(self);
                exclusive_waiters_.InsertTail(&self->WaitListEntry);
                exclusive_waiters_count_++;
                if (exclusive_waiters_count_ > max_exclusive_waiters_count_)
                    max_exclusive_waiters_count_ = exclusive_waiters_count_;
            }
        }
        mu_lock_.Unlock();

        Coroutine::Yield();
    }
}

void CoRwMutex::ReadLock()
{
    BUG_ON(!Coroutine::InCoroutine());

    auto self = Coroutine::Self();
    for (;;) {
        mu_lock_.Lock();
        if (owner_ == nullptr) {
            shared_owner_count_++;
            Sync::Coroutine::Get(self);
            mu_lock_.Unlock();
            return;
        } else {
            if (self->WaitListEntry.IsEmpty()) {
                Sync::Coroutine::Get(self);
                shared_waiters_.InsertTail(&self->WaitListEntry);
                shared_waiters_count_++;
                if (shared_waiters_count_ > max_shared_waiters_count_)
                    max_shared_waiters_count_ = shared_waiters_count_;

            }
        }
        mu_lock_.Unlock();

        Coroutine::Yield();
    }
}

void CoRwMutex::Unlock()
{
    BUG_ON(!Coroutine::InCoroutine());

    auto self = Coroutine::Self();
    mu_lock_.Lock();
    if (shared_owner_count_ == 0) {
        BUG_ON(self != owner_);
        owner_ = nullptr;
        WakeupWaiters(true);
        WakeupWaiters(false);
    } else {
        BUG_ON(owner_ != nullptr);
        shared_owner_count_--;
        if (shared_owner_count_ == 0) {
            WakeupWaiters(false);
            WakeupWaiters(true);
        } else {
            WakeupWaiters(true);
            WakeupWaiters(false);
        }
    }
    mu_lock_.Unlock();

    Sync::Coroutine::Put(self);
}

ulong CoRwMutex::GetMaxExclusiveWaitersCount()
{
    return max_exclusive_waiters_count_;
}

ulong CoRwMutex::GetMaxSharedWaitersCount()
{
    return max_shared_waiters_count_;
}

}