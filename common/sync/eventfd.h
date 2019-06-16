#pragma once

#include <common/base.h>
#include <common/stdlib/result.h>
#include <common/stdlib/error.h>

namespace Sync
{

class Eventfd
{
public:
    Eventfd();

    Stdlib::Error Create(unsigned int init_value, int flags);

    Stdlib::Result<u64, Stdlib::Error> Read();

    Stdlib::Error Write(u64 value);

    int GetFd();

    void Close();

    virtual ~Eventfd();

    static const int kCLOEXEC = 0x1;
    static const int kNONBLOCK = 0x2;
    static const int kSEMAPHORE = 0x4;

private:
    Eventfd(const Eventfd& other) = delete;
    Eventfd(Eventfd&& other) = delete;
    Eventfd& operator=(const Eventfd& other) = delete;
    Eventfd& operator=(Eventfd&& other) = delete;
    int fd_;
};

}