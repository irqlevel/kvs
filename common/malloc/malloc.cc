#include "malloc.h"

#include <malloc.h>
#include <stdlib.h>
#include <memory.h>

struct malloc_header {
    unsigned long magic;
    size_t size;
};

#define MALLOC_HEADER_MAGIC 0xCBDACBDFUL

void* common_malloc(size_t size)
{
    malloc_header *header;

    header = static_cast<malloc_header *>(malloc(sizeof(*header) + size));
    if (header == nullptr)
        abort();

    header->magic = MALLOC_HEADER_MAGIC;
    header->size = size;
    return header + 1;
}

void common_free(void *ptr)
{
    malloc_header *header = static_cast<malloc_header *>(ptr);
    header--;
    if (header->magic != MALLOC_HEADER_MAGIC)
        abort();

    auto size = sizeof(*header) + header->size;
    memset(header, 0xCC, size);
    free(header);
}

void* operator new(size_t size)
{
    return common_malloc(size);
}

void* operator new[](size_t size)
{
    return common_malloc(size);
}

void operator delete(void* ptr)
{
    common_free(ptr);
}

void operator delete[](void* ptr)
{
    common_free(ptr);
}

void operator delete(void* ptr, size_t size)
{
    (void)size;
    common_free(ptr);
}

void operator delete[](void* ptr, size_t size)
{
    (void)size;
    common_free(ptr);
}

void* operator new(size_t size, void *ptr)
{
    (void)size;
    return ptr;
}

void* operator new[](size_t size, void *ptr)
{
    (void)size;
    return ptr;
}