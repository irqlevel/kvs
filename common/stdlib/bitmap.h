#pragma once

#include <common/base.h>

namespace Stdlib
{

class Bitmap
{
public:
    Bitmap(void* address_, ulong bit_count_);
    ~Bitmap();

    void SetBit(ulong position);
    void ClearBit(ulong position);
    bool TestBit(ulong position);
    long FindSetZeroBit();

private:
    Bitmap(const Bitmap& other) = delete;
    Bitmap(Bitmap&& other) = delete;
    Bitmap& operator=(const Bitmap& other) = delete;
    Bitmap& operator=(Bitmap&& other) = delete;

    void* address_;
    ulong bit_count_;
};

}