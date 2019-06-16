#include "file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <common/stdlib/errno.h>

#include <errno.h>

namespace FileSys
{

File::File()
    : fd_(-1)
{
}

Stdlib::Error File::Open(const char* path)
{
    fd_ = ::open(path, O_RDONLY, 0666);
    if (fd_ < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
    return 0;
}

Stdlib::Error File::Open(const Stdlib::String &path)
{
    return Open(path.GetConstBuf());
}

File::~File()
{
    Close();
}

Stdlib::Result<size_t, Stdlib::Error> File::Read(void* buf, size_t nbytes)
{
    ssize_t r = ::read(fd_, buf, nbytes);
    if (r < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    return Stdlib::Result<size_t, Stdlib::Error>(r, 0);
}

Stdlib::Result<size_t, Stdlib::Error>  File::Write(const void *buf, size_t nbytes)
{
    ssize_t r = ::write(fd_, buf, nbytes);
    if (r < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    return Stdlib::Result<size_t, Stdlib::Error>(r, 0);
}

void File::Close()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

Stdlib::Error File::Sync()
{
    int r = ::fsync(fd_);
    if (r < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    return 0;
}

int File::GetFd()
{
    return fd_;
}

Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> File::ReadAll()
{
    if (fd_ < 0)
        return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERRNO_ERROR(EBADFD));

    Stdlib::ByteArray<u8> data;
    Stdlib::ByteArray<u8> buf;
    if (!buf.ReserveAndUse(512 * 1024))
        return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERRNO_ERROR(ENOMEM));

    for (;;) {
        auto result = Read(buf.GetBuf(), buf.GetSize());
        if (result.Error())
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(result.Error());

        auto readBytes = result.Value();
        if (readBytes == 0)
            break;

        if (!data.Append(buf.GetConstBuf(), readBytes))
            return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(STDLIB_ERRNO_ERROR(ENOMEM));
    }

    return Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error>(data, 0);
}

}