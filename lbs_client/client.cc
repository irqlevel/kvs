#include "client.h"
#include <lbs/service.pb.h>

namespace Lbs
{
    #define LbsClientErrNoMemory     STDLIB_ERROR(1, "lbs-client", "no memory")


    Stdlib::Error Client::ResponseHeaderToError(response_header &resp_header)
    {
        if (resp_header.error[0] == '\0')
            return 0;

        Trace(0, "error %s\n", resp_header.error);
        return STDLIB_ERROR(1, "lbs", "internal error");        
    }

    Stdlib::Result<Stdlib::String, Stdlib::Error> Client::AddDisk(const Stdlib::String name, s64 block_size)
    {
        auto req = Stdlib::MakeUnique<add_disk_request>();
        if (req.Get() == nullptr)
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(LbsClientErrNoMemory);

        Stdlib::SnPrintf(req->name, Stdlib::ArraySize(req->name), "%s", name.GetConstBuf());
        req->block_size = block_size;
        auto result = Send<add_disk_request, add_disk_response>(request_type_add_disk, add_disk_request_fields, req, add_disk_response_fields);
        if (result.Error())
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(result.Error());

        auto& resp = result.Value();
        auto err = ResponseHeaderToError(resp->header);
        if (err)
            return err;

        Stdlib::String disk_id(resp->disk_id);

        if (disk_id.GetLength() != Stdlib::StrLen(resp->disk_id))
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(LbsClientErrNoMemory);

        return Stdlib::Result<Stdlib::String, Stdlib::Error>(Stdlib::Move(disk_id), Stdlib::Move(0));
    }

    Stdlib::Result<s64, Stdlib::Error> Client::WriteDisk(const Stdlib::String& disk_id, s64 offset, Stdlib::ByteArray<u8> &data)
    {
        auto req = Stdlib::MakeUnique<write_disk_request>();
        if (req.Get() == nullptr)
            return Stdlib::Result<s64, Stdlib::Error>(LbsClientErrNoMemory);

        req->offset = offset;
        if (sizeof(req->data.bytes) < data.GetSize())
            return Stdlib::Result<s64, Stdlib::Error>(LbsClientErrNoMemory);

        Stdlib::MemCpy(req->data.bytes, data.GetConstBuf(), data.GetSize());
        req->data.size = data.GetSize();
        Stdlib::SnPrintf(req->disk_id, Stdlib::ArraySize(req->disk_id), "%s", disk_id.GetConstBuf());

        auto result = Send<write_disk_request, write_disk_response>(request_type_write_disk, write_disk_request_fields, req, write_disk_response_fields);
        if (result.Error())
            return Stdlib::Result<s64, Stdlib::Error>(result.Error());

        auto& resp = result.Value();
        auto err = ResponseHeaderToError(resp->header);
        if (err)
            return err;

        return Stdlib::Result<s64, Stdlib::Error>(resp->bytes_written, 0);
    }

    Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> Client::ReadDisk(const Stdlib::String& disk_id, s64 offset, size_t size)
    {
        auto req = Stdlib::MakeUnique<read_disk_request>();
        if (req.Get() == nullptr)
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(LbsClientErrNoMemory);

        req->offset = offset;
        req->size = size;
        Stdlib::SnPrintf(req->disk_id, Stdlib::ArraySize(req->disk_id), "%s", disk_id.GetConstBuf());

        auto result = Send<read_disk_request, read_disk_response>(request_type_read_disk, read_disk_request_fields, req, read_disk_response_fields);
        if (result.Error())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(result.Error());

        auto& resp = result.Value();
        auto err = ResponseHeaderToError(resp->header);
        if (err)
            return err;

        Stdlib::ByteArray<u8> data;
        if (!data.ReserveAndUse(resp->data.size))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(LbsClientErrNoMemory);

        Stdlib::MemCpy(data.GetBuf(), resp->data.bytes, resp->data.size);
        return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(Stdlib::Move(data), 0);
    }

    Stdlib::Error Client::SyncDisk(const Stdlib::String& disk_id)
    {
        auto req = Stdlib::MakeUnique<sync_disk_request>();
        if (req.Get() == nullptr)
            return LbsClientErrNoMemory;

        Stdlib::SnPrintf(req->disk_id, Stdlib::ArraySize(req->disk_id), "%s", disk_id.GetConstBuf());

        auto result = Send<sync_disk_request, sync_disk_response>(request_type_sync_disk, sync_disk_request_fields, req, sync_disk_response_fields);
        if (result.Error())
            return result.Error();

        auto& resp = result.Value();
        auto err = ResponseHeaderToError(resp->header);
        if (err)
            return err;

        return 0;
    }
}