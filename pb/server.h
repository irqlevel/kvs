#pragma once

#include <common/sync/coroutine.h>
#include <common/sync/corwmutex.h>
#include <common/stdlib/hash_map.h>
#include <common/stdlib/bytearray.h>
#include <common/net/tcp_req_server.h>

#include "interface.h"

namespace Pb
{

class Server : public Net::TcpReqServer
{
public:

    Server();

    virtual ~Server();

    virtual Stdlib::Error HandleRequest(Stdlib::UniquePtr<TcpReqServer::Request>& request, Stdlib::UniquePtr<TcpReqServer::Response>& response) override;

    template <class PbRequestStruc, class PbResponseStruc>
    Stdlib::Error RegisterHandler(int type, const pb_field_t *pb_request_fields, const pb_field_t *pb_response_fields, 
                        Stdlib::Error (*pb_handle_request)(Stdlib::UniquePtr<PbRequestStruc> &req, Stdlib::UniquePtr<PbResponseStruc> &resp))
    {
        if (!Handlers.ReserveAndUse(type + 1))
            return STDLIB_ERROR(1, "pb", "no memory");
    
        auto handler = new PbRequestHandler<PbRequestStruc, PbResponseStruc>(type, pb_request_fields, pb_response_fields, pb_handle_request);
        if (handler == nullptr)
            return STDLIB_ERROR(1, "pb", "no memory");

        Handlers[type].Reset(handler);
        return 0;
    }

    void Shutdown();

private:
    Server(const Server& other) = delete;
    Server(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;
    Server& operator=(Server&& other) = delete;

    class RequestHandler
    {
    public:
        virtual ~RequestHandler() {}
        virtual Stdlib::Error HandleRequest(Stdlib::ByteArray<u8> &request_payload, Stdlib::ByteArray<u8> &response_payload) = 0;
    };

    template <class PbRequestStruc, class PbResponseStruc>
    class PbRequestHandler : public RequestHandler
    {
    public:
        PbRequestHandler(int pb_type, const pb_field_t *pb_request_fields, const pb_field_t *pb_response_fields,
                        Stdlib::Error (*pb_handle_request)(Stdlib::UniquePtr<PbRequestStruc> &req, Stdlib::UniquePtr<PbResponseStruc> &resp))
            : _pb_type(pb_type), _pb_request_fields(pb_request_fields), _pb_response_fields(pb_response_fields), _pb_handle_request(pb_handle_request)
        {
        }

        virtual ~PbRequestHandler()
        {
        }

        virtual Stdlib::Error HandleRequest(Stdlib::ByteArray<u8> &request_payload, Stdlib::ByteArray<u8> &response_payload) override
        {
            auto req = Stdlib::MakeUnique<PbRequestStruc>();
            auto resp = Stdlib::MakeUnique<PbResponseStruc>();
            if (req.Get() == nullptr || resp.Get() == nullptr)
                return STDLIB_ERROR(1, "pb", "no memory"); 

            auto err = Pb::Decode(request_payload, _pb_request_fields, req.Get());
            if (err)
                return err;

            err = _pb_handle_request(req, resp);
            if (err)
                return err;

            auto result = Pb::Encode(_pb_response_fields, resp.Get());
            if (result.Error())
                return err;

            response_payload = Stdlib::Move(result.Value());
            return 0;     
        }
    private:
        int _pb_type;
        const pb_field_t *_pb_request_fields;
        const pb_field_t *_pb_response_fields;
        Stdlib::Error (*_pb_handle_request)(Stdlib::UniquePtr<PbRequestStruc> &req, Stdlib::UniquePtr<PbResponseStruc> &resp);
    };

    Stdlib::Vector<Stdlib::UniquePtr<RequestHandler>> Handlers;
};

}