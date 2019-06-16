#include "base.h"

#include "errno.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

namespace Stdlib
{

void *MemAdd(void *ptr, unsigned long len)
{
    return reinterpret_cast<void *>(reinterpret_cast<unsigned long>(ptr) + len);
}

const void *MemAdd(const void *ptr, unsigned long len)
{
    return reinterpret_cast<const void *>(reinterpret_cast<unsigned long>(ptr) + len);
}

void MemSet(void* ptr, unsigned char c, size_t size)
{
    ::memset(ptr, c, size);
}

int MemCmp(const void* ptr1, const void* ptr2, size_t size)
{
    return ::memcmp(ptr1, ptr2, size);
}

void MemCpy(void* dst, const void* src, size_t size)
{
    ::memcpy(dst, src, size);
}

void MemMove(void* dst, const void* src, size_t size)
{
    ::memmove(dst, src, size);
}

size_t StrLen(const char* s)
{
    return ::strlen(s);
}

void ByteSwap(u8 *b1, u8 *b2)
{
    u8 tmp = *b1;
    *b1 = *b2;
    *b2 = tmp;
}

void MemReverse(void *mem, size_t size)
{
    u8 *p_mem = (u8 *)mem;
    size_t i;

    for (i = 0; i < size/2; i++)
        ByteSwap(&p_mem[i], &p_mem[size - i - 1]);
}

bool PutChar(char c, char *dst, size_t dst_size, size_t pos)
{
    if (pos >= dst_size)
        return false;
    dst[pos] = c;
    return true;
}

char GetDecDigit(u8 val)
{
    if (val >= 10)
        return '\0';
    return '0' + val;
}

char GetHexDigit(u8 val)
{
    if (val >= 16)
        return '\0';

    if (val < 10)
        return GetDecDigit(val);
    return 'A' + (val - 10);
}

int VsnPrintf(char *s, size_t size, const char *fmt, va_list arg)
{
    return ::vsnprintf(s, size, fmt, arg);
}

int SnPrintf(char* buf, size_t size, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int len = VsnPrintf(buf, size, fmt, args);
    va_end(args);

    return len;
}

int StrCmp(const char *s1, const char *s2)
{
    return ::strcmp(s1, s2);
}

int StrnCmp(const char *s1, const char *s2, size_t size)
{
    return ::strncmp(s1, s2, size);
}

void StrnCpy(char *dst, const char *src, size_t size)
{
    ::strncpy(dst, src, size);
}

// find n : 2^n >= size
size_t Log2(size_t size)
{
    if (size < 2)
        return 0;
    else {
        size_t log = 0;
        size_t rest_size = size;
        while (rest_size != 0)
        {
            rest_size >>= 1;
            log++;
        }

        return (size & (size - 1)) ? log : (log - 1);
    }
}

const char* StrChrOnce(const char* s, char sep)
{
    const char* curr = s;
    const char* res = nullptr;

    for (;;)
    {
        if (*curr == '\0')
        {
            return res;
        }

        if (*curr == sep)
        {
            if (res == nullptr)
            {
                res = curr;
            }
            else
            {
                return nullptr;
            }
        }
        curr++;
    }
}

bool IsValueInRange(ulong value, ulong base, ulong limit)
{
    if (value >= base && value < limit)
        return true;

    return false;
}


bool BufEqualString(const char *buf, size_t buf_len, const char *str)
{
    if (buf_len != StrLen(str) || StrnCmp(buf, str, buf_len) != 0)
        return false;

    return true;
}

Result<ulong, Stdlib::Error> Str2Ulong(const char *s, int base)
{
    char *endptr = nullptr;

    Errno::Reset(0);
    ulong value = ::strtoul(s, &endptr, base);
    int err = Errno::Get();
    if (err)
        return Result<ulong, Stdlib::Error>(STDLIB_ERRNO_ERROR(err));

    if (endptr == nullptr || endptr == s)
        return Result<ulong, Stdlib::Error>(STDLIB_ERRNO_ERROR(EINVAL));
    
    if (*endptr != '\0')
        return Result<ulong, Stdlib::Error>(STDLIB_ERRNO_ERROR(EINVAL));

    return Result<ulong, Stdlib::Error>(value, 0);
}

}
