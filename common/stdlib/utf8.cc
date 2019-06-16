#include "utf8.h"
#include "trace.h"

namespace Stdlib
{

static u32 g_OffsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

#define ISUTF(c) (((c)&0xC0)!=0x80)

Result<Stdlib::String, Stdlib::Error> Utf8::Encode(int codepoint)
{
    Result<Stdlib::String, Stdlib::Error> result;

    if (codepoint < 0)
    {
        result.SetError(STDLIB_ERRNO_ERROR(EINVAL));
    }
    else if(codepoint < 0x80)
    {
        if (!result.MutValue().Append((char)codepoint)) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
    }
    else if(codepoint < 0x800)
    {
        if (!result.MutValue().Append(0xC0 + ((codepoint & 0x7C0) >> 6))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
        if (!result.MutValue().Append(0x80 + ((codepoint & 0x03F)))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
    }
    else if(codepoint < 0x10000)
    {
        if (!result.MutValue().Append(0xE0 + ((codepoint & 0xF000) >> 12))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
        if (!result.MutValue().Append(0x80 + ((codepoint & 0x0FC0) >> 6))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
        if (!result.MutValue().Append(0x80 + ((codepoint & 0x003F)))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
    }
    else if(codepoint <= 0x10FFFF)
    {
        if (!result.MutValue().Append(0xF0 + ((codepoint & 0x1C0000) >> 18))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
        if (!result.MutValue().Append(0x80 + ((codepoint & 0x03F000) >> 12))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
        if (!result.MutValue().Append(0x80 + ((codepoint & 0x000FC0) >> 6))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
        if (!result.MutValue().Append(0x80 + ((codepoint & 0x00003F)))) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
    }
    else
    {
        result.SetError(STDLIB_ERRNO_ERROR(EINVAL));
    }

    return result;
}

Result<Stdlib::String, Stdlib::Error> Utf8::DecodeFromEscapedUtf8(const Stdlib::String& s)
{
    return DecodeFromEscapedUtf8(s.GetConstBuf(), s.GetLength());
}

Result<Stdlib::String, Stdlib::Error> Utf8::DecodeFromEscapedUtf8(const char *s, size_t len)
{
    Stdlib::String output;

    for (size_t i = 0; i < len;) {
        if (s[i] == '\\' && ((i + 1) < len) && s[i+1] == 'u') {
            char tmp[5];

            for (int j = 0; j < 4; j++) {
                if ((i + 2 + j) >= len)
                    return Result<Stdlib::String, Stdlib::Error>(STDLIB_ERRNO_ERROR(EINVAL));

                tmp[j] = s[i + 2 + j];
                if (!String::IsHexDigit(tmp[j]))
                    return Result<Stdlib::String, Stdlib::Error>(STDLIB_ERRNO_ERROR(EINVAL));
            }

            tmp[4] = '\0';
            auto result = Str2Ulong(tmp, 16);
            if (result.Error())
                return Result<Stdlib::String, Stdlib::Error>(result.Error());

            auto result2 = Encode(result.Value());
            if (result2.Error())
                return Result<Stdlib::String, Stdlib::Error>(result2.Error());

            if (!output.Append(result2.Value()))
                return Result<Stdlib::String, Stdlib::Error>(STDLIB_ERRNO_ERROR(ENOMEM));

            i+= 6;
        }
        else
        {
            if (!output.Append(s[i]))
                return Result<Stdlib::String, Stdlib::Error>(STDLIB_ERRNO_ERROR(ENOMEM));

            i+= 1;
        }
    }

    return Result<Stdlib::String, Stdlib::Error>(output, 0);
}

u32 Utf8::NextChar(const char *s, size_t *i)
{
    u32 ch = 0;
    size_t sz = 0;

    do {
        ch <<= 6;
        ch += (unsigned char)s[(*i)];
        sz++;
    } while (s[*i] && (++(*i)) && !ISUTF(s[*i]));
    ch -= g_OffsetsFromUTF8[sz-1];

    return ch;
}

Result<Stdlib::String, Stdlib::Error> Utf8::EncodeToEscapedUtf8(const Stdlib::String &s)
{
    u32 code;
    size_t i;
    Result<Stdlib::String, Stdlib::Error> result;

    i = 0;
    for (;;) {
        code = NextChar(s.GetConstBuf(), &i);
        if (code == 0)
            break;

        char tmp[7];
        if (Stdlib::SnPrintf(tmp, 7, "\\u%04x", code) != 6) {
            result.SetError(STDLIB_ERRNO_ERROR(EINVAL));
            return result;
        }
        tmp[6] = '\0';

        if (!result.MutValue().Append(tmp, 6)) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }
    }

    if (i != s.GetLength()) {
        result.SetError(STDLIB_ERRNO_ERROR(EINVAL));
        return result;
    }

    return result;
}

}