#pragma once

#include <common/base.h>
#include <pthread.h>

#include <common/sync/lockable.h>

namespace Sync
{

class RWMutex : public RwLockable
{
public:
    RWMutex();
    virtual ~RWMutex();
    virtual void Lock() override;
    virtual void Unlock() override;
    virtual void ReadLock() override;
private:
    RWMutex(const RWMutex& other) = delete;
    RWMutex(RWMutex&& other) = delete;
    RWMutex& operator=(const RWMutex& other) = delete;
    RWMutex& operator=(RWMutex&& other) = delete;

    pthread_rwlock_t lock_;    
};

}
