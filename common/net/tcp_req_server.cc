#include "tcp_req_server.h"

#include <common/stdlib/vector.h>
#include <common/stdlib/trace.h>
#include <common/stdlib/unique_ptr.h>
#include <common/sync/autolock.h>
#include <common/sync/process.h>
#include <common/stdlib/byteorder.h>

namespace Net
{
    #define ErrNoMemory     STDLIB_ERROR(1, "server", "no memory")
    #define ErrPartialRead  STDLIB_ERROR(2, "server", "partial read")
    #define ErrPartialWrite STDLIB_ERROR(3, "server", "partial write")
    #define ErrInvalidProtocolVersion STDLIB_ERROR(4, "server", "invalid protocol version")

    TcpReqServer::TcpReqServer()
    {
    }

    TcpReqServer::~TcpReqServer()
    {
    }

    Stdlib::Error TcpReqServer::ReadRequest(Net::TcpServer::Connection *conn, Stdlib::UniquePtr<Request> &request, bool& closed)
    {
        closed = false;

        auto result = conn->Read(&request->header_, sizeof(request->header_));
        if (result.Error()) {
            Trace(0, "read error %d\n", result.Error().Code());
            return result.Error();
        }

        if (result.Value() == 0) {
            closed = true;
            return 0;
        }

        if (result.Value() != sizeof(request->header_)) {
            Trace(0, "partial read\n");
            return ErrPartialRead;
        }

        request->header_.protocol_version_ = Stdlib::ByteOrder::NtohU32(request->header_.protocol_version_);
        if (request->header_.protocol_version_ != kProtocolVersion) {
            Trace(0, "invalid protocol version\n");
            return ErrInvalidProtocolVersion;
        }

        request->header_.payload_length_ = Stdlib::ByteOrder::NtohU32(request->header_.payload_length_);
        if (request->header_.payload_length_ > kMaxRequestPayloadSize) {
            Trace(0, "no memory\n");
            return ErrNoMemory;
        }

        if (!request->payload_.ReserveAndUse(request->header_.payload_length_)) {
            Trace(0, "no memory\n");
            return ErrNoMemory;
        }

        result = conn->Read(request->payload_.GetBuf(), request->payload_.GetSize());
        if (result.Error()) {
            Trace(0, "read error %d\n", result.Error().Code());
            return result.Error();
        }

        if (result.Value() != request->payload_.GetSize()) {
            Trace(0, "partial read %lu vs. %lu\n", result.Value(), request->payload_.GetSize());
            return ErrPartialRead;
        }

        return 0;
    }

    Stdlib::Error TcpReqServer::WriteResponse(Net::TcpServer::Connection *conn, Stdlib::UniquePtr<Response> &response)
    {
        response->header_.protocol_version_ = Stdlib::ByteOrder::HtonU32(kProtocolVersion);
        response->header_.payload_length_ = Stdlib::ByteOrder::HtonU32(response->payload_.GetSize());
        auto result = conn->Write(&response->header_, sizeof(response->header_));
        if (result.Error()) {
            Trace(0, "write error %d\n", result.Error().Code());
            return result.Error();
        }

        if (result.Value() != sizeof(response->header_)) {
            Trace(0, "partial write\n");
            return ErrPartialWrite;
        }

        result = conn->Write(response->payload_.GetBuf(), response->payload_.GetSize());
        if (result.Error()) {
            Trace(0, "write error %d\n", result.Error().Code());
            return result.Error();
        }

        if (result.Value() != response->payload_.GetSize()) {
            Trace(0, "partial write\n");
            return ErrPartialWrite;
        }

        return 0;
    }

    void TcpReqServer::ConnectionCoHandler(Net::TcpServer::Connection *conn)
    {
        for (;;) {
            auto request = Stdlib::MakeUnique<Request>();
            auto response = Stdlib::MakeUnique<Response>();

            if (request.Get() == nullptr || response.Get() == nullptr) {
                Trace(0, "no memory\n");
                return;
            }

            bool closed;
            auto err = ReadRequest(conn, request, closed);
            if (err) {
                Trace(0, "read request error %d\n", err.Code());
                return;
            }
            if (closed)
                return;

            err = HandleRequest(request, response);
            if (err) {
                Trace(0, "handle request error %d\n", err.Code());
                return;
            }

            err = WriteResponse(conn, response);
            if (err) {
                Trace(0, "write response error %d\n", err.Code());
                return;
            }

        }
    }


    TcpReqServer::Client::Client()
    {
    }

    TcpReqServer::Client::~Client()
    {
    }

    Stdlib::Error TcpReqServer::Client::Connect(const char *address, int port)
    {
        socket_.Reset();
        socket_ = Stdlib::MakeUnique<Net::Socket>();

        if (socket_.Get() == nullptr)
            return STDLIB_ERROR(1, "client", "no memory");

        return socket_->Connect(address, port);
    }

    void TcpReqServer::Client::Close()
    {
        socket_.Reset();
    }

    Stdlib::UniquePtr<Net::Socket>& TcpReqServer::Client::GetSocket()
    {
        return socket_;
    }

    Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> TcpReqServer::Client::SendRequest(const Stdlib::ByteArray<u8>& request)
    {
        if (socket_.Get() == nullptr || socket_->GetFd() < 0)
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERROR(1, "client", "no open socket"));

        RequestHeader request_header;
        request_header.protocol_version_ = Stdlib::ByteOrder::HtonU32(kProtocolVersion);
        request_header.payload_length_ = Stdlib::ByteOrder::HtonU32(request.GetSize());
        auto result = socket_->Write(&request_header, sizeof(request_header));
        if (result.Error())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(result.Error());
        if (result.Value() != sizeof(request_header))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERROR(2, "client", "partial write"));

        result = socket_->Write(request.GetConstBuf(), request.GetSize());
        if (result.Error())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(result.Error());

        if (result.Value() != request.GetSize())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERROR(2, "client", "partial write"));

        ResponseHeader response_header;
        result = socket_->Read(&response_header, sizeof(response_header));
        if (result.Error())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(result.Error());

        if (result.Value() != sizeof(response_header))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERROR(3, "client", "partial read"));

        response_header.payload_length_ = Stdlib::ByteOrder::NtohU32(response_header.payload_length_);
        response_header.protocol_version_ = Stdlib::ByteOrder::NtohU32(response_header.protocol_version_);
        if (response_header.protocol_version_ != kProtocolVersion)
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERROR(4, "client", "invalid protocol version"));

        Stdlib::ByteArray<u8> response;
        if (!response.ReserveAndUse(response_header.payload_length_))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERROR(5, "client", "no memory"));

        result = socket_->Read(response.GetBuf(), response.GetSize());
        if (result.Error())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(result.Error());

        if (result.Value() != response.GetSize())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERROR(3, "client", "partial read"));

        return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(Move(response), 0);
    }
}