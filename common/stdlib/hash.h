#pragma once

#include <common/base.h>
#include "bytearray.h"

namespace Stdlib
{

size_t Djb2Hash(const void* address, size_t length, size_t hash = 5381);

size_t HashPtr(const void* address);

size_t HashInt(const int& value);

size_t HashIntAdd(const int& value, size_t hash = 5381);

size_t HashBytes(const ByteArray<u8>& bytes);

}