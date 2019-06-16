#include "socket.h"

#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>

#include <common/stdlib/trace.h>
#include <common/stdlib/errno.h>
#include <common/sync/coroutine.h>

namespace Net
{

static const int SocketLL = 6;

Socket::Socket()
    : fd_(-1)
{
}

Socket::Socket(int fd)
    : fd_(fd)
{
}

int Socket::GetFd()
{
    return fd_;
}

Stdlib::Error Socket::MakeFdNonBlocking(int fd)
{
    int flags, r;

    flags = ::fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    flags |= O_NONBLOCK;
    r = ::fcntl(fd, F_SETFL, flags);
    if (r == -1)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    return 0;
}

Stdlib::Error Socket::Bind(const char *addr, int port)
{
    int val;
    Stdlib::Error err;
    int fd;
    struct sockaddr_in saddr;

    if (fd_ != -1)
        return STDLIB_ERRNO_ERROR(EBADFD);

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = ::inet_addr(addr);
    saddr.sin_port = htons(port);

    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    val = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant set reuseport accept\n");
        goto fail;
    }

    val = 1;
    if (::setsockopt(fd, SOL_TCP, TCP_QUICKACK, &val, sizeof(val)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant set tcp quick ack\n");
        goto fail;
    }

    val = 1;
    if (::setsockopt(fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant set tcp no delay\n");
        goto fail;
    }

    if (::bind(fd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant bind\n");
        goto fail;
    }

    err = MakeFdNonBlocking(fd);
    if (err) {
        Trace(SocketLL, "cant make fd non blocking\n");
        goto fail;
    }

    fd_ = fd;
    return 0;

fail:
    ::close(fd);
    return err;
}

Stdlib::Error Socket::Connect(const char *addr, int port)
{
    int val;
    Stdlib::Error err;
    int fd;
    struct sockaddr_in saddr;

    if (fd_ != -1)
        return STDLIB_ERRNO_ERROR(EBADFD);

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = ::inet_addr(addr);
    saddr.sin_port = htons(port);

    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    val = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant set reuseport accept\n");
        goto fail;
    }

    val = 1;
    if (::setsockopt(fd, SOL_TCP, TCP_QUICKACK, &val, sizeof(val)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant set tcp quick ack\n");
        goto fail;
    }

    val = 1;
    if (::setsockopt(fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant set tcp no delay\n");
        goto fail;
    }

    if (::connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
        err = STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        Trace(SocketLL, "cant bind\n");
        goto fail;
    }

    err = MakeFdNonBlocking(fd);
    if (err) {
        Trace(SocketLL, "cant make fd non blocking error %d\n", err.Code());
        goto fail;
    }

    fd_ = fd;
    return 0;

fail:
    ::close(fd);
    return err;
}

void Socket::Close()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

Stdlib::Result<Socket*, Stdlib::Error> Socket::Accept()
{
    struct sockaddr saddr;
    socklen_t saddr_len;
    int val;
    Stdlib::Result<Socket*, Stdlib::Error> result;

    if (fd_ == -1)
        return Stdlib::Result<Socket*, Stdlib::Error>(STDLIB_ERRNO_ERROR(EBADFD));

    saddr_len = sizeof(saddr);
    int fd = ::accept(fd_, &saddr, &saddr_len);
    if (fd == -1) {
        return Stdlib::Result<Socket*, Stdlib::Error>(STDLIB_ERRNO_ERROR(Stdlib::Errno::Get()));
    }

    result.SetError(MakeFdNonBlocking(fd));
    if (result.Error()) {
        Trace(SocketLL, "make fd nonblocking error %d\n", result.Error().Code());
        goto fail;
    }

    val = 1;
    if (::setsockopt(fd, SOL_TCP, TCP_QUICKACK, &val, sizeof(val)) == -1) {
        result.SetError(STDLIB_ERRNO_ERROR(Stdlib::Errno::Get()));
        Trace(SocketLL, "cant set tcp quick ack\n");
        goto fail;
    }

    val = 1;
    if (::setsockopt(fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val)) == -1) {
        result.SetError(STDLIB_ERRNO_ERROR(Stdlib::Errno::Get()));
        Trace(SocketLL, "cant set tcp no delay\n");
        goto fail;
    }

    result.SetValue(new Socket(fd));
    if (result.Value() == nullptr) {
        result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
        goto fail;
    }

    return result;

fail:
    ::close(fd);
    return result;
}

Socket::~Socket()
{  
    if (fd_ != -1)
        ::close(fd_);
}

Stdlib::Result<size_t, Stdlib::Error> Socket::Recv(void* buf, size_t nbytes)
{
    if (fd_ == -1)
        return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(EBADFD));

    ssize_t r = ::recv(fd_, buf, nbytes, 0);
    if (r < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    return Stdlib::Result<size_t, Stdlib::Error>(r, 0);
}

Stdlib::Result<size_t, Stdlib::Error> Socket::Send(const void *buf, size_t nbytes)
{
    if (fd_ == -1)
        return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(EBADFD));

    ssize_t r = ::send(fd_, buf, nbytes, 0);
    if (r < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
    
    return Stdlib::Result<size_t, Stdlib::Error>(r, 0);
}

Stdlib::Error Socket::Listen()
{
    int r = ::listen(fd_, SOMAXCONN);
    if (r < 0) {
        r = Stdlib::Errno::Get();
        Trace(SocketLL, "listen error %d\n", r);
        return STDLIB_ERRNO_ERROR(r);
    }
    return 0;
}

Stdlib::Result<size_t, Stdlib::Error> Socket::Read(void* buf, size_t buf_size)
{
    size_t total_read;

    total_read = 0;
    while (total_read < buf_size) {
        auto result = Recv((char*)buf + total_read, buf_size - total_read);

        if (result.Error()) {
            if (result.Error() == STDLIB_ERRNO_ERROR(EAGAIN) || result.Error() == STDLIB_ERRNO_ERROR(EINTR)) {
                if (Sync::Coroutine::InCoroutine()) {
                    Sync::Coroutine::Yield();
                    continue;
                } else
                    return Stdlib::Result<size_t, Stdlib::Error>(total_read, result.Error());
            } else {
                return Stdlib::Result<size_t, Stdlib::Error>(total_read, result.Error());
            }
        }

        auto read = result.Value();
        total_read+= read;
        if (read == 0)
            break;
    }

    return Stdlib::Result<size_t, Stdlib::Error>(total_read, 0);
}

Stdlib::Result<size_t, Stdlib::Error> Socket::Write(const void* buf, size_t buf_size)
{
    size_t total_written;

    total_written = 0;
    while (total_written < buf_size) {
        auto result = Send((char*)buf + total_written, buf_size - total_written);

        if (result.Error()) {
            if (result.Error() == STDLIB_ERRNO_ERROR(EAGAIN) || result.Error() == STDLIB_ERRNO_ERROR(EINTR)) {
                if (Sync::Coroutine::InCoroutine()) {
                    Sync::Coroutine::Yield();
                    continue;
                } else
                    return Stdlib::Result<size_t, Stdlib::Error>(total_written, result.Error());
            } else
                return Stdlib::Result<size_t, Stdlib::Error>(total_written, result.Error());
        }

        auto written = result.Value();
        if (written == 0)
            return STDLIB_ERRNO_ERROR(EPIPE);

        total_written+= written;
    }

    return Stdlib::Result<size_t, Stdlib::Error>(total_written, 0);
}

}
