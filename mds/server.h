#pragma once

#include <common/sync/coroutine.h>
#include <common/sync/corwmutex.h>
#include <common/stdlib/hash_map.h>
#include <common/stdlib/bytearray.h>
#include <common/net/tcp_req_server.h>

namespace Mds
{

class Server : public Net::TcpReqServer
{
public:

    static const int kEchoRequestType = 1;

    static Server& GetInstance() {
        static Server g_server;
        return g_server;
    }

    virtual Stdlib::Error HandleRequest(Stdlib::UniquePtr<TcpReqServer::Request>& request, Stdlib::UniquePtr<TcpReqServer::Response>& response) override;

    virtual ~Server();

    static void OnStopSignal(int signo);

    static void OnSigPipe(int signo);

private:
    Server();

    Server(const Server& other) = delete;
    Server(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;
    Server& operator=(Server&& other) = delete;

    Sync::Atomic stop_signal_pending_;
};

}