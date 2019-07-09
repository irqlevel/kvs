#pragma once

#include <unistd.h>
#include <stddef.h>
#include <stdarg.h>

#include <common/stdlib/abort.h>

typedef unsigned long ulong;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

static_assert(sizeof(char) == 1, "invalid size");

static_assert(sizeof(u8) == 1, "invalid size");
static_assert(sizeof(u16) == 2, "invalid size");
static_assert(sizeof(u32) == 4, "invalid size");
static_assert(sizeof(u64) == 8, "invalid size");

static_assert(sizeof(s8) == 1, "invalid size");
static_assert(sizeof(s16) == 2, "invalid size");
static_assert(sizeof(s32) == 4, "invalid size");
static_assert(sizeof(s64) == 8, "invalid size");

#define OFFSET_OF(type, field)	\
		(unsigned long)&((type*)0)->field

#define CONTAINING_RECORD(addr, type, field)	\
            (type*)((unsigned long)(addr) - OFFSET_OF(type, field))

#define UNREFERENCED_PARAMETER(p)   \
            (void)(p)

#define likely(x)       __builtin_expect((x),1)

#define unlikely(x)     __builtin_expect((x),0)

#define BUG_ON(condition)   \
        (unlikely(condition)) ? Stdlib::Abort(__FILE__, __LINE__, __FUNCTION__) : false

const size_t kPageSize = 4096;