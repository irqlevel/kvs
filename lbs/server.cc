#include "server.h"

#include <common/stdlib/vector.h>
#include <common/stdlib/trace.h>
#include <common/stdlib/unique_ptr.h>
#include <common/sync/autolock.h>
#include <common/sync/process.h>
#include <common/stdlib/byteorder.h>


#include <pb/interface.h>
#include "service.pb.h"
#include "error.h"

namespace Lbs
{
    Server::Server()
    {
        RegisterHandler<echo_request, echo_response>(request_type_echo, echo_request_fields, echo_response_fields, &Server::Echo);
        RegisterHandler<add_disk_request, add_disk_response>(request_type_add_disk, add_disk_request_fields, add_disk_response_fields, &Server::AddDisk);
        RegisterHandler<write_disk_request, write_disk_response>(request_type_write_disk, write_disk_request_fields, write_disk_response_fields, &Server::WriteDisk);
        RegisterHandler<read_disk_request, read_disk_response>(request_type_read_disk, read_disk_request_fields, read_disk_response_fields, &Server::ReadDisk);
        RegisterHandler<sync_disk_request, sync_disk_response>(request_type_sync_disk, sync_disk_request_fields, sync_disk_response_fields, &Server::SyncDisk);
    }

    Server::~Server()
    {
    }

    Stdlib::Error Server::EchoInternal(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp)
    {
        if (req->data.size > sizeof(req->data.bytes))
            return MakeErrorResponse(LbsErrInvalidRequest, resp->header);

        if (req->data.size > sizeof(resp->data.bytes))
            return MakeErrorResponse(LbsErrInvalidRequest, resp->header);

        Stdlib::MemCpy(resp->data.bytes, req->data.bytes, req->data.size);
        resp->data.size = req->data.size;
        return 0;
    }

    Stdlib::Error Server::Echo(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp) {
        return Server::GetInstance().Echo(req, resp);
    }

    Stdlib::Error Server::MakeErrorResponse(Stdlib::Error err, response_header &resp)
    {
        Stdlib::SnPrintf(resp.error, Stdlib::ArraySize(resp.error), "%s: %d %s", err.GetSubsystem(), err.Code(), err.GetMessage());
        return 0;
    }

    Stdlib::Error Server::AddDiskInternal(Stdlib::UniquePtr<add_disk_request> &req, Stdlib::UniquePtr<add_disk_response> &resp)
    {
        Stdlib::String name;
        if (!name.Append(req->name, Stdlib::StrLen(req->name)))
            return MakeErrorResponse(LbsErrInvalidRequest, resp->header);

        auto result = disk_manager_.AddDisk(name, req->size, req->block_size);
        if (result.Error())
            return MakeErrorResponse(result.Error(), resp->header);

        auto disk_id = result.Value();
        Stdlib::SnPrintf(resp->disk_id, Stdlib::ArraySize(resp->disk_id), "%s", disk_id.GetConstBuf());
        return 0;
    }

    Stdlib::Error Server::AddDisk(Stdlib::UniquePtr<add_disk_request> &req, Stdlib::UniquePtr<add_disk_response> &resp) {
        return Server::GetInstance().AddDiskInternal(req, resp);
    }

    Stdlib::Error Server::WriteDiskInternal(Stdlib::UniquePtr<write_disk_request> &req, Stdlib::UniquePtr<write_disk_response> &resp)
    {
        Stdlib::String disk_id;
        if (!disk_id.Append(req->disk_id, Stdlib::StrLen(req->disk_id)))
            return MakeErrorResponse(LbsErrInvalidRequest, resp->header);

        auto result = disk_manager_.LookupDisk(disk_id);
        if (result.Error())
            return MakeErrorResponse(result.Error(), resp->header);

        auto disk = result.Value();

        auto result2 = disk->Write(req->offset, req->data.bytes, req->data.size);
        if (result2.Error())
            return MakeErrorResponse(result2.Error(), resp->header);

        resp->bytes_written = result2.Value();
        return 0;
    }

    Stdlib::Error Server::WriteDisk(Stdlib::UniquePtr<write_disk_request> &req, Stdlib::UniquePtr<write_disk_response> &resp) {
        return Server::GetInstance().WriteDiskInternal(req, resp);
    }

    Stdlib::Error Server::SyncDiskInternal(Stdlib::UniquePtr<sync_disk_request> &req, Stdlib::UniquePtr<sync_disk_response> &resp)
    {
        Stdlib::String disk_id;
        if (!disk_id.Append(req->disk_id, Stdlib::StrLen(req->disk_id)))
            return MakeErrorResponse(LbsErrInvalidRequest, resp->header);

        auto result = disk_manager_.LookupDisk(disk_id);
        if (result.Error())
            return MakeErrorResponse(result.Error(), resp->header);

        auto disk = result.Value();
        auto err = disk->Sync();
        if (err)
            return MakeErrorResponse(err, resp->header);

        return 0;
    }

    Stdlib::Error Server::SyncDisk(Stdlib::UniquePtr<sync_disk_request> &req, Stdlib::UniquePtr<sync_disk_response> &resp) {
        return Server::GetInstance().SyncDiskInternal(req, resp);
    }

    Stdlib::Error Server::ReadDiskInternal(Stdlib::UniquePtr<read_disk_request> &req, Stdlib::UniquePtr<read_disk_response> &resp)
    {
        if (req->size > static_cast<s64>(sizeof(resp->data.bytes)))
            return MakeErrorResponse(LbsErrInvalidRequest, resp->header);

        Stdlib::String disk_id;
        if (!disk_id.Append(req->disk_id, Stdlib::StrLen(req->disk_id)))
            return MakeErrorResponse(LbsErrInvalidRequest, resp->header);

        auto result = disk_manager_.LookupDisk(disk_id);
        if (result.Error())
            return MakeErrorResponse(result.Error(), resp->header);

        auto disk = result.Value();
        auto result2 = disk->Read(req->offset, resp->data.bytes, req->size);
        if (result2.Error())
            return MakeErrorResponse(result2.Error(), resp->header);

        resp->data.size = result2.Value();
        return 0;
    }

    Stdlib::Error Server::ReadDisk(Stdlib::UniquePtr<read_disk_request> &req, Stdlib::UniquePtr<read_disk_response> &resp) {
        return Server::GetInstance().ReadDiskInternal(req, resp);
    }

    void Server::OnSigPipe(int signo)
    {
        Trace(0, "signo %d\n", signo);
    }

    void Server::Shutdown()
    {
        Pb::Server::Shutdown();
        disk_manager_.Shutdown();
    }

    void Server::OnStopSignal(int signo)
    {
        Trace(0, "signo %d\n", signo);

        auto &server = Server::GetInstance();
        if (server.stop_signal_pending_.CmpXchg(0, 1) == 0)
        {
            Server::GetInstance().Shutdown();
            Trace(0, "exit memusage %lu\n", get_mem_usage());
            Sync::Process::Exit(0);
        }
    }
}