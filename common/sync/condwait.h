#pragma once


#include <common/base.h>
#include <pthread.h>
#include "mutex.h"

namespace Sync
{

class CondWait
{
public:
    CondWait();
    virtual ~CondWait();

    using CondFunction = bool (*)(void* context);

    virtual void Wait(Mutex& lock, CondFunction func, void* context);
    virtual void Signal(Mutex& lock);
    virtual void Broadcast(Mutex& lock);

private:
    CondWait(const CondWait& other) = delete;
    CondWait(CondWait&& other) = delete;
    CondWait& operator=(const CondWait& other) = delete;
    CondWait& operator=(CondWait&& other) = delete;

    pthread_cond_t cond_wait_;
};

}