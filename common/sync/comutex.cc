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
    BUG_ON(owner_.Get() != nullptr);
    WakeupWaiters();
    mu_lock_.Unlock();
}

void CoMutex::WakeupWaiters()
{
    while (!waiters_.IsEmpty()) {
        auto co_list_entry = CONTAINING_RECORD(waiters_.RemoveHead(), CoroutineListEntry, list_entry_);
        co_list_entry->list_entry_.Init();
        co_list_entry->co_->Signal();
        delete co_list_entry;
    }
}

void CoMutex::Lock()
{
    BUG_ON(!Coroutine::InCoroutine());

    auto self = Coroutine::Self();

    for (;;) {
        mu_lock_.Lock();
        if (owner_.Get() == nullptr) {
            owner_ = self;
            mu_lock_.Unlock();
            return;
        } else {
            auto le = new CoroutineListEntry(self);
            waiters_.InsertTail(&le->list_entry_);
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
    BUG_ON(self.Get() != owner_.Get());
    owner_.Reset(nullptr);
    WakeupWaiters();
    mu_lock_.Unlock();
}

}