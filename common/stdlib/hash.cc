#include "hash.h"

namespace Stdlib
{

size_t Djb2Hash(const void* address, size_t length, size_t hash)
{
    for (size_t index = 0; index < length; index++)
        hash = ((hash << 5) + hash) + ((unsigned char *)address)[index];

    return hash;
}

size_t HashPtr(const void* address)
{
    ulong value = (ulong)address;
    return Djb2Hash(&value, sizeof(value));
}

size_t HashIntAdd(const int& value, size_t hash)
{
    return Djb2Hash((void*)&value, sizeof(value), hash);
}

size_t HashInt(const int& value)
{
    return Djb2Hash((void*)&value, sizeof(value));
}

size_t HashBytes(const ByteArray<u8>& bytes)
{
    return Djb2Hash(bytes.GetConstBuf(), bytes.GetSize());
}

}