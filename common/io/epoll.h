#pragma once

#include <common/base.h>
#include <common/io.h>
#include <common/stdlib/bytearray.h>
#include <common/stdlib/error.h>

namespace IO
{

class Epoll
{
public:
    struct Event {
        int fd_;
        int flags_;
    };

    Epoll();
    virtual ~Epoll();
    Stdlib::Error Create();
    void Close();

    static const int kIN = 0x1;
    static const int kOUT = 0x2;
    static const int kERR = 0x4;
    static const int kHUP = 0x8;
    static const int kET = 0x10;
    static const int kONESHOT = 0x20;

    Stdlib::Error Add(int fd, int flags);
    Stdlib::Error Mod(int fd, int flags);
    Stdlib::Error Del(int fd);
    Stdlib::Error Wait(void* buf, size_t buf_size, Stdlib::ByteArray<Event> &events);

    int GetFd();

private:
    Epoll(const Epoll& other) = delete;
    Epoll(Epoll&& other) = delete;
    Epoll& operator=(const Epoll& other) = delete;
    Epoll& operator=(Epoll&& other) = delete;

    int fd_;
};

}
