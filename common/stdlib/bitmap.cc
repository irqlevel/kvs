#include "bitmap.h"
#include "base.h"

namespace Stdlib
{

Bitmap::Bitmap(void* address, ulong bit_count)
    : address_(address)
    , bit_count_(bit_count)
{
    BUG_ON((ulong)address_ % sizeof(ulong));
}

Bitmap::~Bitmap()
{
}

void Bitmap::SetBit(ulong position)
{
    BUG_ON(position >= bit_count_);

    ulong offset = position / (8 * sizeof(ulong));
    ulong shift = position % (8 * sizeof(ulong));

    *((ulong *)address_ + offset) |= (1UL << shift);
}

void Bitmap::ClearBit(ulong position)
{
    BUG_ON(position >= bit_count_);

    ulong offset = position / (8 * sizeof(ulong));
    ulong shift = position % (8 * sizeof(ulong));

    *((ulong *)address_ + offset) &= ~(1UL << shift);    
}

bool Bitmap::TestBit(ulong position)
{
    BUG_ON(position >= bit_count_);

    ulong offset = position / (8 * sizeof(ulong));
    ulong shift = position % (8 * sizeof(ulong));

    if ((*((ulong *)address_ + offset)) & (1UL << shift))
        return true;
    return false;
}

long Bitmap::FindSetZeroBit()
{
    ulong* curr = (ulong*)address_;
    long restbit_count_ = bit_count_;

    while (restbit_count_ > 0)
    {
        ulong value = *curr;
        if (value != ~0UL)
        {
            for (ulong shift = 0; shift < Min<ulong>(8 * sizeof(ulong), restbit_count_); shift++)
            {
                if (!(value & (1UL << shift)))
                {
                    *curr |= (1UL << shift);
                    long position = 8 * ((ulong)curr - (ulong)address_) + shift;
                    BUG_ON((ulong)position >= bit_count_);
                    return position;
                }
            }
        }
        restbit_count_ -= 8 * sizeof(ulong);
        curr++;
    }
    return -1;
}

}