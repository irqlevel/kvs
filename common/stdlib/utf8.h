#pragma once

#include "base.h"

#include "string.h"
#include "result.h"

namespace Stdlib
{
    class Utf8 {
    public:
        static Result<Stdlib::String, Stdlib::Error> DecodeFromEscapedUtf8(const Stdlib::String &s);
        static Result<Stdlib::String, Stdlib::Error> DecodeFromEscapedUtf8(const char *s, size_t len);
        static Result<Stdlib::String, Stdlib::Error> EncodeToEscapedUtf8(const Stdlib::String &s);
    private:
        static Result<Stdlib::String, Stdlib::Error> Encode(int codepoint);
        static u32 NextChar(const char *s, size_t *i);
    };
}