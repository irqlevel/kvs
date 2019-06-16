#include "error.h"
#include "base.h"

namespace Stdlib
{

const char *TruncateFileName(const char *file_name)
{
    const char *base, *last_sep = nullptr;

    base = file_name;
    for (;;)
    {
        if (*base == '\0')
            break;

        if (*base == '/')
        {
            last_sep = base;
        }
        base++;
    }

    if (last_sep)
        return last_sep + 1;
    else
        return file_name;
}

Error::Error()
    : func_("unknown")
    , file_("unknown")
    , message_("unknown")
    , subsystem_("unknown")
    , line_(-1)
    , code_(0)
{
}

Error::Error(int code)
    : Error()
{
    code_ = code;
}

Error::Error(int code, const char *subsystem, const char *message, const char *func, const char *file, int line)
    : Error()
{
    code_ = code;
    message_ = message;
    subsystem_ = subsystem;
    func_ = func;
    file_ = file;
    line_ = line;
}

Error::~Error()
{
}

int Error::Code() const
{
    return code_;
}

void Error::SetCode(int code)
{
    code_ = code;
}

bool Error::operator!= (const Error& other) const
{
    return (code_ != other.code_ || subsystem_ != other.subsystem_);
}

bool Error::operator== (const Error& other) const
{
    return (code_ == other.code_ && Stdlib::StrCmp(subsystem_, other.subsystem_) == 0);
}

void Error::Reset()
{
    code_ = 0;
    subsystem_ = "unwknown";
    message_ = "unknown";
    file_ = "unknown";
    func_ = "unknown";
    line_ = -1;
}

int Error::GetLine() const
{
    return line_;
}

const char* Error::GetFile() const
{
    return file_;
}

const char* Error::GetFunc() const
{
    return func_;
}

Error::Error(const Error& other)
    : Error()
{
    code_ = other.code_;
    message_ = other.message_;
    subsystem_ = other.subsystem_;
    func_ = other.func_;
    file_ = other.file_;
    line_ = other.line_;
}

Error::Error(Error&& other)
{
    code_ = other.code_;
    message_ = other.message_;
    subsystem_ = other.subsystem_;
    func_ = other.func_;
    file_ = other.file_;
    line_ = other.line_;
    other.Reset();
}

Error& Error::operator=(const Error& other)
{
    if (this != &other) {
        code_ = other.code_;
        message_ = other.message_;
        func_ = other.func_;
        file_ = other.file_;
        line_ = other.line_;
    }
    return *this;
}

Error& Error::operator=(Error&& other)
{
    if (this != &other) {
        code_ = other.code_;
        message_ = other.message_;
        subsystem_ = other.subsystem_;
        func_ = other.func_;
        file_ = other.file_;
        line_ = other.line_;
        other.Reset();
    }
    return *this;
}

Error::operator bool() const
{
    return code_ != 0;
}

}