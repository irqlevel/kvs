#include "server.h"

#include <common/stdlib/vector.h>
#include <common/stdlib/trace.h>
#include <common/stdlib/unique_ptr.h>
#include <common/sync/autolock.h>
#include <common/sync/process.h>
#include <common/stdlib/byteorder.h>

namespace Mds
{
    #define ErrNoMemory     STDLIB_ERROR(1, "mds", "no memory")
    #define ErrInvalidRequestType STDLIB_ERROR(2, "mds", "invalid request type")

    Server::Server()
    {
    }

    Server::~Server()
    {
    }

    void Server::OnSigPipe(int signo)
    {
        Trace(0, "signo %d\n", signo);
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

    Stdlib::Error Server::HandleRequest(Stdlib::UniquePtr<Net::TcpReqServer::Request>& request, Stdlib::UniquePtr<Net::TcpReqServer::Response>& response)
    {
        switch (request->header_.type_) {
        case kEchoRequestType: {
            if (!response->payload_.CopyFrom(request->payload_))
                return ErrNoMemory;
            return 0;
        }
        default:
            return ErrInvalidRequestType;
        }
        return 0;
    }
}