#pragma once

#include <common/net/tcp_req_server.h>
#include <common/sync/process.h>
#include <common/stdlib/trace.h>
#include <common/io/random.h>
#include <common/stdlib/unique_ptr.h>
#include <common/stdlib/error.h>
#include <pb/client.h>
#include <lbs/service.pb.h>

namespace Lbs
{
    class Client : public Pb::Client
    {
    public:
        Stdlib::Result<Stdlib::String, Stdlib::Error> AddDisk(const Stdlib::String name, s64 block_size);
        Stdlib::Result<s64, Stdlib::Error> WriteDisk(const Stdlib::String& disk_id, s64 offset, Stdlib::ByteArray<u8> &data);
        Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> ReadDisk(const Stdlib::String& disk_id, s64 offset, size_t size);
        Stdlib::Error SyncDisk(const Stdlib::String& disk_id);
    private:
        Stdlib::Error ResponseHeaderToError(response_header &resp_header);
    };
}