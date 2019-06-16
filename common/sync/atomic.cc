#include "atomic.h"

namespace Sync
{

Atomic::Atomic()
{
    Set(0);
}

Atomic::Atomic(long value)
{
    Set(value);
}

Atomic::~Atomic()
{
}

void Atomic::Set(long value)
{
    value_ = value;
}

long Atomic::Add(long delta)
{
    return __sync_add_and_fetch(&value_, delta);
}

long Atomic::Sub(long delta)
{
    return __sync_sub_and_fetch(&value_, delta);
}

long Atomic::Inc()
{
    return Add(1);
}

long Atomic::Dec()
{
    return Sub(1);
}

long Atomic::CmpXchg(long old_value, long new_value)
{
    return __sync_val_compare_and_swap(&value_, old_value, new_value);
}

long Atomic::Get()
{
    return __sync_fetch_and_add(&value_, 0);
}

}