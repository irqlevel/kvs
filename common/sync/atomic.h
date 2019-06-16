#pragma once

#include <common/base.h>

namespace Sync
{

class Atomic 
{
public:
    Atomic();
    Atomic(long value);
    virtual ~Atomic();

    void Set(long value);
    long Get();
    long Inc();
    long Dec();
    long CmpXchg(long old_value, long new_value);
    long Add(long delta);
    long Sub(long delta);
private:
    Atomic(const Atomic& other) = delete;
    Atomic(Atomic&& other) = delete;
    Atomic& operator=(const Atomic& other) = delete;
    Atomic& operator=(Atomic&& other) = delete;

    long value_;
};

}