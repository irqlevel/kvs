#pragma once

#include <common/base.h>
#include <common/stdlib/base.h>

void* common_malloc(size_t size);
void common_free(void *ptr);

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* ptr);
void operator delete[](void* ptr);

void operator delete(void* ptr, size_t size);
void operator delete[](void* ptr, size_t size);

void* operator new(size_t size, void *ptr);
void* operator new[](size_t size, void *ptr);

template<typename T, class... Args>
T* TAlloc(Args&&... args)
{
    void* p = common_malloc(sizeof(T));
    if (p == nullptr)
        return nullptr;

    return new (p) T(Stdlib::Forward<Args>(args)...);
}

long get_mem_usage();