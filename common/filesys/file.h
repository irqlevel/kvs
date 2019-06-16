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
    File();
    Stdlib::Error Open(const char *path);
    Stdlib::Error Open(const Stdlib::String& path);
    virtual ~File();
    virtual Stdlib::Result<size_t, Stdlib::Error> Read(void* buf, size_t nbytes) override;
    virtual Stdlib::Result<size_t, Stdlib::Error> Write(const void *buf, size_t nbytes) override;
    virtual void Close() override;
    virtual Stdlib::Error Sync();
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