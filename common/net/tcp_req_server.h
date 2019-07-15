#pragma once

#include <common/stdlib/bytearray.h>
#include <common/net/tcp_server.h>

namespace Net
{

class TcpReqServer : public Net::TcpServer
{
public:

    static const u32 kProtocolVersion = 0xcbdacbda;
    static const size_t kMaxRequestPayloadSize = 1024 * 1024;

    struct RequestHeader {
        u32 protocol_version_;
        u32 payload_length_;
        u32 type_;
    };

    struct Request {
        RequestHeader header_;
        Stdlib::ByteArray<u8> payload_;
    };

    struct ResponseHeader {
        u32 protocol_version_;
        u32 payload_length_;
    };

    struct Response {
        struct ResponseHeader header_;
        Stdlib::ByteArray<u8> payload_;
    };

    TcpReqServer();
    virtual ~TcpReqServer();

    virtual void ConnectionCoHandler(Net::TcpServer::Connection *conn) override;

    virtual Stdlib::Error HandleRequest(Stdlib::UniquePtr<Request>& request, Stdlib::UniquePtr<Response>& response) = 0;

    class Client {
    public:
        Client();
        Stdlib::Error Connect(const char *address, int port, bool non_blocking = true);
        Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> SendRequest(u32 type, const Stdlib::ByteArray<u8>& request);
        void Close();
        virtual ~Client();
        Stdlib::UniquePtr<Net::Socket>& GetSocket();
    private:
        Stdlib::UniquePtr<Net::Socket> socket_;
    };

private:
    TcpReqServer(const TcpReqServer& other) = delete;
    TcpReqServer(TcpReqServer&& other) = delete;
    TcpReqServer& operator=(const TcpReqServer& other) = delete;
    TcpReqServer& operator=(TcpReqServer&& other) = delete;

    Stdlib::Error ReadRequest(Net::TcpServer::Connection *conn, Stdlib::UniquePtr<Request> &request, bool& closed);

    Stdlib::Error WriteResponse(Net::TcpServer::Connection *conn, Stdlib::UniquePtr<Response> &response);


};

}