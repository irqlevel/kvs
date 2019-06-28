#include "server.h"

namespace Pb
{
    #define ErrNoMemory     STDLIB_ERROR(1, "pb", "no memory")
    #define ErrInvalidRequestType STDLIB_ERROR(2, "pb", "invalid request type")

    Server::Server()
    {
    }

    Server::~Server()
    {
    }

    void Server::Shutdown()
    {
        TcpReqServer::Shutdown();
        Handlers.Clear();
    }

    Stdlib::Error Server::HandleRequest(Stdlib::UniquePtr<Net::TcpReqServer::Request>& request, Stdlib::UniquePtr<Net::TcpReqServer::Response>& response)
    {

        if (request->header_.type_ < Handlers.GetSize()) {
            auto handler = Handlers[request->header_.type_].Get();
            if (handler != nullptr)
                return handler->HandleRequest(request->payload_, response->payload_);
        }

        return ErrInvalidRequestType;
    }
}