#include "comutex.h"
#include "coroutine.h"

namespace Sync
{

CoMutex::CoMutex()
    : owner_(nullptr)
{
}

CoMutex::~CoMutex()
{
    mu_lock_.Lock();
    BUG_ON(owner_ != nullptr);
    WakeupWaiters();
    mu_lock_.Unlock();
}

void CoMutex::WakeupWaiters()
{
    while (!waiters_.IsEmpty()) {
        auto co = CONTAINING_RECORD(waiters_.RemoveHead(), Coroutine, WaitListEntry);
        co->WaitListEntry.Init();
        co->Signal();
        Sync::Coroutine::Put(co);
    }
}

void CoMutex::Lock()
{
    BUG_ON(!Coroutine::InCoroutine());

    auto self = Coroutine::Self();

    for (;;) {
        mu_lock_.Lock();
        if (owner_ == nullptr) {
            Sync::Coroutine::Get(self);
            owner_ = self;
            mu_lock_.Unlock();
            return;
        } else {
            if (self->WaitListEntry.IsEmpty()) {
                Sync::Coroutine::Get(self);
                waiters_.InsertTail(&self->WaitListEntry);
            }
        }
        mu_lock_.Unlock();

        Coroutine::Yield();
    }
}

void CoMutex::Unlock()
{
    BUG_ON(!Coroutine::InCoroutine());

    auto self = Coroutine::Self();

    mu_lock_.Lock();
    BUG_ON(self != owner_);
    owner_ = nullptr;
    WakeupWaiters();
    mu_lock_.Unlock();
    Sync::Coroutine::Put(self);
}

}