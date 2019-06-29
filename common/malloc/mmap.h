#pragma once

#include <common/base.h>
#include <common/stdlib/base.h>

namespace Memory
{


class Mmap
{
public:
    Mmap();
    ~Mmap();

    Stdlib::Error Map(size_t buf_size);
    void Unmap();

    void *GetBuf();
    size_t GetSize();

private:
    void *_buf;
    size_t _buf_size;
};

};