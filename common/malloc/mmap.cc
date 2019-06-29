#include "mmap.h"
#include <sys/mman.h>
#include <errno.h>
#include <common/stdlib/errno.h>

namespace Memory
{

Mmap::Mmap()
    : _buf(nullptr), _buf_size(0)
{
}

Mmap::~Mmap()
{
    if (_buf != nullptr)
        Unmap();
}

Stdlib::Error Mmap::Map(size_t buf_size)
{
    if (_buf)
        return STDLIB_ERRNO_ERROR(EINVAL);

    auto result = ::mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    _buf = result;
    _buf_size = buf_size;

    return 0;
}

void *Mmap::GetBuf()
{
    return _buf;
}

size_t Mmap::GetSize()
{
    return _buf_size;
}

void Mmap::Unmap()
{
    ::munmap(_buf, _buf_size);
    _buf = nullptr;
    _buf_size = 0;
}

}