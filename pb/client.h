#pragma once


#include "interface.h"
#include <common/net/tcp_req_server.h>

namespace Pb
{

class Client : public Net::TcpReqServer::Client
{
public:
    template <class PbRequestStruc, class PbResponseStruc>
    Stdlib::Result<Stdlib::UniquePtr<PbResponseStruc>, Stdlib::Error> Send(int type, const pb_field_t *req_fields, Stdlib::UniquePtr<PbRequestStruc> &req, const pb_field_t *resp_fields)
    {
        //Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> Encode(const pb_field_t pb_fields[], void *pb_struc);
        //Stdlib::Error Decode(const Stdlib::ByteArray<u8> &data, const pb_field_t fields[], void *struc);

        auto result = Encode(req_fields, req.Get());
        if (result.Error())
            return Stdlib::Result<Stdlib::UniquePtr<PbResponseStruc>, Stdlib::Error>(result.Error());


        auto result2 = SendRequest(type, result.Value());
        if (result2.Error())
            return Stdlib::Result<Stdlib::UniquePtr<PbResponseStruc>, Stdlib::Error>(result.Error());

        auto resp = Stdlib::MakeUnique<PbResponseStruc>();
        auto err = Decode(result2.Value(), resp_fields, resp.Get());
        if (err)
            return Stdlib::Result<Stdlib::UniquePtr<PbResponseStruc>, Stdlib::Error>(err);

        return Stdlib::Result<Stdlib::UniquePtr<PbResponseStruc>, Stdlib::Error>(Stdlib::Move(resp), 0);
    }
};

}