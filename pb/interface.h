#pragma once

#include "pb_encode.h"
#include "pb_decode.h"

#include <common/stdlib/hash_map.h>
#include <common/stdlib/bytearray.h>

namespace Pb
{
    Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> Encode(const pb_field_t pb_fields[], void *pb_struc);

    Stdlib::Error Decode(const Stdlib::ByteArray<u8> &data, const pb_field_t fields[], void *struc);

}