#pragma once

#include <common/base.h>
#include "bytearray.h"

namespace Stdlib
{

class Bitmap
{
public:
    Bitmap();
    virtual ~Bitmap();

    bool Alloc(ulong bit_count);
    void ClearAll();
    void SetBit(ulong position);
    void ClearBit(ulong position);
    bool TestBit(ulong position);
    long FindSetZeroBit();

    size_t GetSize();

    void* GetBuf();

private:
    Bitmap(const Bitmap& other) = delete;
    Bitmap(Bitmap&& other) = delete;
    Bitmap& operator=(const Bitmap& other) = delete;
    Bitmap& operator=(Bitmap&& other) = delete;

    Stdlib::ByteArray<byte> bitmap_;
    ulong *GetBase();

    ulong bit_count_;
};

}