#include "server.h"

#include <common/stdlib/vector.h>
#include <common/stdlib/trace.h>
#include <common/stdlib/unique_ptr.h>
#include <common/sync/autolock.h>
#include <common/sync/process.h>
#include <common/stdlib/byteorder.h>


#include <pb/interface.h>
#include "service.pb.h"

namespace Mds
{
    #define ErrNoMemory     STDLIB_ERROR(1, "mds", "no memory")
    #define ErrInvalidRequestType STDLIB_ERROR(2, "mds", "invalid request type")
    #define ErrInvalidRequest STDLIB_ERROR(3, "mds", "invalid request")

    Server::Server()
    {
        RegisterHandler<echo_request, echo_response>(request_type_echo, echo_request_fields, echo_response_fields, &Server::HandleEcho);
    }

    Server::~Server()
    {
    }

    Stdlib::Error Server::HandleEchoInternal(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp)
    {
        if (req->data.size > sizeof(req->data.bytes))
            return ErrInvalidRequest;
        
        if (req->data.size > sizeof(resp->data.bytes))
            return ErrInvalidRequest;

        Stdlib::MemCpy(resp->data.bytes, req->data.bytes, req->data.size);
        resp->data.size = req->data.size;
        return 0;
    }

    Stdlib::Error Server::HandleEcho(Stdlib::UniquePtr<echo_request> &req, Stdlib::UniquePtr<echo_response> &resp) {
        return Server::GetInstance().HandleEchoInternal(req, resp);
    }

    void Server::OnSigPipe(int signo)
    {
        Trace(0, "signo %d\n", signo);
    }

    void Server::Shutdown()
    {
        Pb::Server::Shutdown();
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