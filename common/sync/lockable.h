#pragma once

#include <common/base.h>

namespace Sync
{

class Lockable
{
public:
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};

class RwLockable : public Lockable
{
public:
    virtual void ReadLock() = 0;
};

}
