#pragma once

#include <common/base.h>
#include <common/io.h>

#include <common/stdlib/error.h>

namespace IO
{
    class AioContext
    {
    public:

        static const int kRead = 0x0;
        static const int kWrite = 0x1;
        static const int kDataSync = 0x2;

        static const int kEventfd = 0x1;

        AioContext();
        virtual ~AioContext();
        
        Stdlib::Error Setup();
        Stdlib::Error Submit(int fd, s64 offset, void *buf, size_t buf_size, int event_fd, int flags, int op);
        Stdlib::Result<s64, Stdlib::Error> Wait();
        Stdlib::Error Destroy();

    private:
        ulong _ctx_id;
    };
}