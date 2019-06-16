#include "epoll.h"

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>

#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include <common/stdlib/base.h>
#include <common/stdlib/errno.h>
#include <common/stdlib/trace.h>

namespace IO
{


Epoll::Epoll()
    : fd_(-1)
{
}

int Epoll::GetFd()
{
    return fd_;
}

Stdlib::Error Epoll::Create()
{
    if (fd_ != -1)
        return STDLIB_ERRNO_ERROR(EBADFD);

    int fd = ::epoll_create1(0);
    if (fd == -1) {
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
    }

    fd_ = fd;
    return 0;
}

void Epoll::Close()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

Stdlib::Error Epoll::Add(int fd, int flags)
{
    if (fd_ == -1)
        return STDLIB_ERRNO_ERROR(EBADFD);

    struct epoll_event event;

    memset(&event, 0, sizeof(event));

    event.data.fd = fd;
    event.events = 0;
    if (flags & Epoll::kIN)
        event.events |= EPOLLIN;
    if (flags & Epoll::kOUT)
        event.events |= EPOLLOUT;
    if (flags & Epoll::kERR)
        event.events |= EPOLLERR;
    if (flags & Epoll::kHUP)
        event.events |= EPOLLHUP;
    if (flags & Epoll::kET)
        event.events |= EPOLLET;
    if (flags & Epoll::kONESHOT)
        event.events |= EPOLLONESHOT;

    Trace(10, "epoll add %d\n", fd);
    int r = ::epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &event);
    if (r < 0) {
        auto err = Stdlib::Errno::Get();
        Trace(10, "epoll add %d err %d\n", fd, err);
        return STDLIB_ERRNO_ERROR(err);
    }

    return 0;
}

Stdlib::Error Epoll::Mod(int fd, int flags)
{
    if (fd_ == -1)
        return STDLIB_ERRNO_ERROR(EBADFD);

    struct epoll_event event;

    memset(&event, 0, sizeof(event));

    event.data.fd = fd;
    event.events = 0;
    if (flags & Epoll::kIN)
        event.events |= EPOLLIN;
    if (flags & Epoll::kOUT)
        event.events |= EPOLLOUT;
    if (flags & Epoll::kERR)
        event.events |= EPOLLERR;
    if (flags & Epoll::kHUP)
        event.events |= EPOLLHUP;
    if (flags & Epoll::kET)
        event.events |= EPOLLET;
    if (flags & Epoll::kONESHOT)
        event.events |= EPOLLONESHOT;

    //Trace(0, "epoll mod %d\n", fd);
    int r = ::epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event);
    if (r < 0) {
        auto err = Stdlib::Errno::Get();
        //Trace(0, "epoll mod %d err %d\n", fd, err);
        return STDLIB_ERRNO_ERROR(err);
    }

    return 0;
}

Stdlib::Error Epoll::Del(int fd)
{
    if (fd_ == -1)
        return STDLIB_ERRNO_ERROR(EBADFD);
 
    struct epoll_event event;

    memset(&event, 0, sizeof(event));

    Trace(10, "epoll del %d\n", fd);
    int r = ::epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &event);
    if (r < 0) {
        auto err = Stdlib::Errno::Get();
        Trace(10, "epoll del %d err %d\n", fd, err);
        return STDLIB_ERRNO_ERROR(err);
    }

    return 0;
}

Stdlib::Error Epoll::Wait(void* buf, size_t buf_size, Stdlib::ByteArray<Event> &events)
{
    if (fd_ == -1)
        return STDLIB_ERRNO_ERROR(EBADFD);

    if (buf == nullptr || buf_size < sizeof(struct epoll_event))
        return STDLIB_ERRNO_ERROR(EINVAL);

    struct epoll_event* rawEvents = static_cast<struct epoll_event*>(buf);
    int r = ::epoll_wait(fd_, rawEvents, buf_size / sizeof(struct epoll_event), -1);
    if (r < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    if (!events.ReserveAndUse(r))
        return STDLIB_ERRNO_ERROR(ENOMEM);

    for (size_t i = 0; i < (size_t)r; i++) {
        events[i].fd_ = rawEvents[i].data.fd;
        events[i].flags_ = rawEvents[i].events;
    }

    return 0;
}

Epoll::~Epoll()
{
    Close();
}



}
