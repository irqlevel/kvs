#include "interface.h"

namespace Pb
{
    #define ErrCantGetEncodedSize STDLIB_ERROR(1, "pb", "can't get encoded size")
    #define ErrNoMemory STDLIB_ERROR(2, "pb", "no memory")
    #define ErrEncode STDLIB_ERROR(3, "pb", "encode error")
    #define ErrDecode STDLIB_ERROR(4, "pb", "decode error")

    Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> Encode(const pb_field_t pb_fields[], void *pb_struc)
    {
        size_t required_size;
        Stdlib::ByteArray<u8> data;

        if (!pb_get_encoded_size(&required_size, pb_fields, pb_struc))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(ErrCantGetEncodedSize);

        if (!data.ReserveAndUse(required_size))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(ErrNoMemory);

        auto stream = pb_ostream_from_buffer(data.GetBuf(), data.GetSize());
        if (!pb_encode(&stream, pb_fields, pb_struc))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(ErrEncode);

        if (!data.Truncate(stream.bytes_written))
            Stdlib::Abort();

        return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(Stdlib::Move(data), 0);
    }

    Stdlib::Error Decode(const Stdlib::ByteArray<u8> &data, const pb_field_t pb_fields[], void *pb_struc)
    {
        auto stream = pb_istream_from_buffer(data.GetConstBuf(), data.GetSize());
        if (!pb_decode(&stream, pb_fields, pb_struc))
            return ErrDecode;

        return 0;
    }



}