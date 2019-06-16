#pragma once

#include <common/base.h>
#include <pthread.h>

#include <common/sync/lockable.h>

namespace Sync
{

class Mutex : public Lockable
{
public:
    Mutex();
    virtual ~Mutex();
    virtual void Lock() override;
    virtual void Unlock() override;
    pthread_mutex_t* GetRawLock();
private:
    Mutex(const Mutex& other) = delete;
    Mutex(Mutex&& other) = delete;
    Mutex& operator=(const Mutex& other) = delete;
    Mutex& operator=(Mutex&& other) = delete;

    pthread_mutex_t lock_;    
};

}