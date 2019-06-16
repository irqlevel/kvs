#include "eventfd.h"

#include <unistd.h>
#include <sys/eventfd.h>

#include <errno.h>

#include <common/stdlib/errno.h>

namespace Sync
{

Eventfd::Eventfd()
    : fd_(-1)
{
}

Stdlib::Error Eventfd::Create(unsigned int init_value, int flags)
{
    if (fd_ != -1)
        return STDLIB_ERRNO_ERROR(EBADFD);

    fd_ = ::eventfd(init_value,
        ((flags & kCLOEXEC) ? EFD_CLOEXEC : 0) |
        ((flags & kNONBLOCK) ? EFD_NONBLOCK : 0) |
        ((flags & kSEMAPHORE) ? EFD_SEMAPHORE : 0));
    if (fd_ == -1)
        return Stdlib::Errno::Get();

    return 0;
}

void Eventfd::Close()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

Stdlib::Result<u64, Stdlib::Error> Eventfd::Read()
{
    u64 value;

    int r = ::read(fd_, &value, sizeof(value));
    if (r == -1)
        return Stdlib::Result<u64, Stdlib::Error>(STDLIB_ERRNO_ERROR(Stdlib::Errno::Get()));

    if (r != sizeof(value))
        return Stdlib::Result<u64, Stdlib::Error>(STDLIB_ERRNO_ERROR(EPIPE));

    return Stdlib::Result<u64, Stdlib::Error>(value, 0);
}

Stdlib::Error Eventfd::Write(u64 value)
{
    int r = ::write(fd_, &value, sizeof(value));
    if (r == -1)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    if (r != sizeof(value))
        return STDLIB_ERRNO_ERROR(EPIPE);

    return 0;
}

int Eventfd::GetFd()
{
    return fd_;
}

Eventfd::~Eventfd()
{
    Close();
}

}