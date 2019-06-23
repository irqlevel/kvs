#include "malloc.h"


#include <malloc.h>
#include <stdlib.h>
#include <memory.h>

#if defined(__DEBUG__)

#include <common/sync/atomic.h>
#include <common/sync/spinlock.h>
#include <common/stdlib/list_entry.h>
#include <common/stdlib/trace.h>
#include <common/stdlib/hash.h>
#include <execinfo.h>

#define MALLOC_LIST_COUNT 512

static Sync::Atomic g_mem_usage;
static Stdlib::ListEntry g_mem_list[MALLOC_LIST_COUNT];
static Sync::SpinLock g_mem_list_lock[MALLOC_LIST_COUNT];

#define MALLOC_HEADER_BACKTRACE_LIMIT 16

struct malloc_header {
    unsigned long magic_;
    size_t size_;
    Stdlib::ListEntry list_entry_;
    void* backtrace_[MALLOC_HEADER_BACKTRACE_LIMIT];
    int backtrace_count_;
};

#define MALLOC_HEADER_MAGIC 0xCBDACBDFUL

void* common_malloc(size_t size)
{
    malloc_header *header;

    header = static_cast<malloc_header *>(malloc(sizeof(*header) + size));
    if (header == nullptr)
        abort();

    header->magic_ = MALLOC_HEADER_MAGIC;
    header->size_ = size;
    header->backtrace_count_ = backtrace(header->backtrace_, MALLOC_HEADER_BACKTRACE_LIMIT);

    int i = Stdlib::HashPtr(header) % MALLOC_LIST_COUNT;
    g_mem_list_lock[i].Lock();
    g_mem_list[i].InsertTail(&header->list_entry_);
    g_mem_list_lock[i].Unlock();

    g_mem_usage.Add(sizeof(*header) + size);
    return header + 1;
}

void common_free(void *ptr)
{
    malloc_header *header = static_cast<malloc_header *>(ptr);
    header--;
    if (header->magic_ != MALLOC_HEADER_MAGIC)
        abort();

    int i = Stdlib::HashPtr(header) % MALLOC_LIST_COUNT;
    g_mem_list_lock[i].Lock();
    header->list_entry_.RemoveInit();
    g_mem_list_lock[i].Unlock();

    auto size = sizeof(*header) + header->size_;
    memset(header, 0xCC, size);
    free(header);
    g_mem_usage.Sub(size);
}

long get_mem_usage()
{
    long usage = g_mem_usage.Get();

    if (usage) {
        for (int i = 0; i < MALLOC_LIST_COUNT; i++) {

            g_mem_list_lock[i].Lock();
            for (auto le = g_mem_list[i].Flink; le != &g_mem_list[i]; le = le->Flink) {
                auto header = CONTAINING_RECORD(le, malloc_header, list_entry_);

                Trace(0, "addr %p size %lu\n", header + 1, header->size_);
                for (int i = 0; i < header->backtrace_count_; i++) {
                    Trace(0, "backtace[%d]=%p\n", i, header->backtrace_[i]);
                }

            }

            g_mem_list_lock[i].Unlock();
        }
    }

    return usage;
}

#else
void* common_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == nullptr)
        abort();

    return ptr;
}

void common_free(void *ptr)
{
    free(ptr);
}

long get_mem_usage()
{
    return 0;
}
#endif

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