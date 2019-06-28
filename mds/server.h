#pragma once

#include <common/sync/coroutine.h>
#include <common/sync/corwmutex.h>
#include <common/stdlib/hash_map.h>
#include <common/stdlib/bytearray.h>
#include <common/net/tcp_req_server.h>

#include <pb/server.h>

#include "service.pb.h"

namespace Mds
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

    Sync::Atomic stop_signal_pending_;

    Stdlib::Error HandleEchoInternal(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp);

    static Stdlib::Error HandleEcho(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp);
};

}