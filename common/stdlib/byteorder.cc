#include "byteorder.h"

#include <arpa/inet.h>
#include <stdio.h>

namespace Stdlib
{

u32 ByteOrder::HtonU32(u32 value)
{
    return htobe32(value);
}

u32 ByteOrder::NtohU32(u32 value)
{
    return be32toh(value);
}

}