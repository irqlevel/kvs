#pragma once

#include <common/sync/coroutine.h>
#include <common/sync/corwmutex.h>
#include <common/stdlib/hash_map.h>
#include <common/stdlib/bytearray.h>
#include <common/net/tcp_req_server.h>

#include <pb/server.h>

#include "service.pb.h"

#include "disk_manager.h"

namespace Lbs
{

class Server : public Pb::Server
{
public:

    static Server& GetInstance() {
        static Server g_server;
        return g_server;
    }

    virtual ~Server();

    static void OnStopSignal(int signo);

    static void OnSigPipe(int signo);

    void Shutdown();

private:
    Server();

    Server(const Server& other) = delete;
    Server(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;
    Server& operator=(Server&& other) = delete;

    Stdlib::Error EchoInternal(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp);

    static Stdlib::Error Echo(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp);


    Stdlib::Error AddDiskInternal(Stdlib::UniquePtr<add_disk_request> &req, Stdlib::UniquePtr<add_disk_response> &resp);

    static Stdlib::Error AddDisk(Stdlib::UniquePtr<add_disk_request> &req, Stdlib::UniquePtr<add_disk_response> &resp);

    Stdlib::Error WriteDiskInternal(Stdlib::UniquePtr<write_disk_request> &req, Stdlib::UniquePtr<write_disk_response> &resp);

    static Stdlib::Error WriteDisk(Stdlib::UniquePtr<write_disk_request> &req, Stdlib::UniquePtr<write_disk_response> &resp);

    Stdlib::Error ReadDiskInternal(Stdlib::UniquePtr<read_disk_request> &req, Stdlib::UniquePtr<read_disk_response> &resp);

    static Stdlib::Error ReadDisk(Stdlib::UniquePtr<read_disk_request> &req, Stdlib::UniquePtr<read_disk_response> &resp);

    Stdlib::Error SyncDiskInternal(Stdlib::UniquePtr<sync_disk_request> &req, Stdlib::UniquePtr<sync_disk_response> &resp);

    static Stdlib::Error SyncDisk(Stdlib::UniquePtr<sync_disk_request> &req, Stdlib::UniquePtr<sync_disk_response> &resp);

    Stdlib::Error MakeErrorResponse(Stdlib::Error err, response_header &resp);

    Sync::Atomic stop_signal_pending_;

    DiskManager disk_manager_;
};

}