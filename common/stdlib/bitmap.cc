#include "bitmap.h"
#include "base.h"

namespace Stdlib
{

Bitmap::Bitmap()
    : bit_count_(0)
{
}

bool Bitmap::Alloc(ulong bit_count)
{
    bitmap_.Clear();
    auto result = bitmap_.ReserveAndUse(Stdlib::RoundUp(Stdlib::RoundUp(bit_count, 8)/8, sizeof(ulong)));    
    if (result)
        bit_count_ = bit_count;

    return result;
}

size_t Bitmap::GetSize()
{
    return bitmap_.GetSize();
}

Bitmap::~Bitmap()
{
}

void Bitmap::ClearAll()
{
    Stdlib::MemSet(bitmap_.GetBuf(), 0, bitmap_.GetSize());
}

ulong *Bitmap::GetBase()
{
    return reinterpret_cast<ulong *>(bitmap_.GetBuf());
}

void* Bitmap::GetBuf()
{
    return bitmap_.GetBuf();
}

void Bitmap::SetBit(ulong position)
{
    BUG_ON(position >= bit_count_);

    ulong offset = position / (BITS_PER_LONG);
    ulong shift = position % (BITS_PER_LONG);

    *(GetBase() + offset) |= (1UL << shift);
}

void Bitmap::ClearBit(ulong position)
{
    BUG_ON(position >= bit_count_);

    ulong offset = position / (BITS_PER_LONG);
    ulong shift = position % (BITS_PER_LONG);

    *(GetBase() + offset) &= ~(1UL << shift);    
}

bool Bitmap::TestBit(ulong position)
{
    BUG_ON(position >= bit_count_);

    ulong offset = position / (BITS_PER_LONG);
    ulong shift = position % (BITS_PER_LONG);

    if ((*(GetBase() + offset)) & (1UL << shift))
        return true;

    return false;
}

long Bitmap::FindSetZeroBit()
{
    ulong* curr = GetBase();
    long rest_bit_count = bit_count_;

    while (rest_bit_count > 0)
    {
        ulong value = *curr;
        if (value != ~0UL)
        {
            for (ulong shift = 0; shift < Min<ulong>(BITS_PER_LONG, rest_bit_count); shift++)
            {
                if (!(value & (1UL << shift)))
                {
                    *curr |= (1UL << shift);
                    long position = 8 * (reinterpret_cast<ulong>(curr) - reinterpret_cast<ulong>(GetBase())) + shift;
                    BUG_ON((ulong)position >= bit_count_);
                    return position;
                }
            }
        }
        rest_bit_count -= BITS_PER_LONG;
        curr++;
    }
    return -1;
}

}