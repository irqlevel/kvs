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

Stdlib::Error File::Open(const char* path, int flags)
{
    int oflags = 0;

    if (flags & kReadOnly)
        oflags |= O_RDONLY;
    if (flags & kDirect)
        oflags |= O_DIRECT;
    if (flags & kSync)
        oflags |= O_SYNC;
    if (flags & kReadWrite)
        oflags |= O_RDWR;

    fd_ = ::open(path, oflags, 0666);
    if (fd_ < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
    return 0;
}

Stdlib::Error File::Open(const Stdlib::String &path, int flags)
{
    return Open(path.GetConstBuf(), flags);
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

Stdlib::Result<size_t, Stdlib::Error>  File::WriteAt(s64 offset, const void *buf, size_t nbytes)
{
    ssize_t r = ::pwrite(fd_, buf, nbytes, offset);
    if (r < 0)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    return Stdlib::Result<size_t, Stdlib::Error>(r, 0);
}

Stdlib::Result<size_t, Stdlib::Error> File::ReadAt(s64 offset, void* buf, size_t nbytes)
{
    ssize_t r = ::pread(fd_, buf, nbytes, offset);
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

Stdlib::Result<s64, Stdlib::Error> File::Seek(u64 offset, int whence)
{
    int wh;

    if (whence == kSeekCur)
        wh = SEEK_CUR;
    else if (whence == kSeekEnd)
        wh = SEEK_END;
    else if (whence == kSeekSet)
        wh = SEEK_SET;
    else
        return Stdlib::Result<s64, Stdlib::Error>(0, STDLIB_ERRNO_ERROR(EINVAL)); 

    s64 result = ::lseek64(fd_, offset, wh);
    if (result == -1LL) {
        int err = Stdlib::Errno::Get();
        return Stdlib::Result<s64, Stdlib::Error>(0, STDLIB_ERRNO_ERROR(err));
    }

    return Stdlib::Result<s64, Stdlib::Error>(result, 0);
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

Stdlib::Error File::DataSync()
{
    int r = ::fdatasync(fd_);
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