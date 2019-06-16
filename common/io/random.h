#pragma once

#include <common/base.h>
#include <common/io.h>
#include <common/stdlib/error.h>
#include <common/stdlib/result.h>

namespace IO
{
    class Random
    {
    public:
        Random();
        virtual ~Random();
        void GetRandomBytes(void *buf, size_t buf_size);
    };
}