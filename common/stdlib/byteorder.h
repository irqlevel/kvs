#pragma once


#include <common/base.h>

namespace Stdlib
{

class ByteOrder
{
public:
    static u32 HtonU32(u32 value);
    static u32 NtohU32(u32 value);
};

}