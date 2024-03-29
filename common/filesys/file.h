#pragma once

#include <common/base.h>
#include <common/io.h>
#include <common/stdlib/bytearray.h>
#include <common/stdlib/result.h>
#include <common/stdlib/string.h>

namespace FileSys
{

class File : public IO::ReadWriteCloser
{
public:

    static const int kReadOnly = 0x1;
    static const int kDirect = 0x2;
    static const int kSync = 0x4;
    static const int kReadWrite = 0x8;

    static const int kSeekSet = 0x1;
    static const int kSeekCur = 0x2;
    static const int kSeekEnd = 0x3;

    File();
    Stdlib::Error Open(const char *path, int flags = kReadOnly);
    Stdlib::Error Open(const Stdlib::String& path, int flags = kReadOnly);
    virtual ~File();

    virtual Stdlib::Result<size_t, Stdlib::Error> Read(void* buf, size_t nbytes) override;
    virtual Stdlib::Result<size_t, Stdlib::Error> Write(const void *buf, size_t nbytes) override;

    Stdlib::Result<size_t, Stdlib::Error> ReadAt(s64 offset, void* buf, size_t nbytes);
    Stdlib::Result<size_t, Stdlib::Error> WriteAt(s64 offset, const void *buf, size_t nbytes);

    Stdlib::Result<s64, Stdlib::Error> Seek(u64 offset, int whence);

    virtual void Close() override;
    virtual Stdlib::Error Sync();
    Stdlib::Error DataSync();

    virtual int GetFd();

    Stdlib::Result<Stdlib::ByteArray<u8>, Stdlib::Error> ReadAll();

private:
    File(const File& other) = delete;
    File(File&& other) = delete;
    File& operator=(const File& other) = delete;
    File& operator=(File&& other) = delete;

    int fd_;
};

}