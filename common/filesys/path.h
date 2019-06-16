#pragma once

#include <common/base.h>
#include <common/io.h>
#include <common/stdlib/string.h>
#include <common/stdlib/result.h>

namespace FileSys
{
    class Path
    {
    public:
        static Stdlib::Result<Stdlib::String, Stdlib::Error> Join(const Stdlib::String &path1, const Stdlib::String &path2);
        static Stdlib::Result<Stdlib::String, Stdlib::Error> GetExtension(const Stdlib::String &path);
    };
}