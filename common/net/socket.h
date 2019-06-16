#pragma once

#include <common/base.h>
#include <common/io.h>

namespace Net
{

class Socket : public IO::ReadWriteCloser
{
public:
    Socket();
    virtual ~Socket();

    Stdlib::Error Connect(const char *addr, int port);
    Stdlib::Error Bind(const char *addr, int port);
    Stdlib::Error Listen();
    Stdlib::Result<Socket*, Stdlib::Error> Accept();

    int GetFd();

    Stdlib::Result<size_t, Stdlib::Error> Read(void* buf, size_t buf_size);
    Stdlib::Result<size_t, Stdlib::Error> Write(const void* buf, size_t buf_size);

    virtual void Close() override;

private:
    Socket(int fd);
    Socket(const Socket& other) = delete;
    Socket(Socket&& other) = delete;
    Socket& operator=(const Socket& other) = delete;
    Socket& operator=(Socket&& other) = delete;

    Stdlib::Result<size_t, Stdlib::Error> Recv(void* buf, size_t nbytes);
    Stdlib::Result<size_t, Stdlib::Error> Send(const void* buf, size_t nbytes);

    static Stdlib::Error MakeFdNonBlocking(int fd);

    int fd_;
};

}