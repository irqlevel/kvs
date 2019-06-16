#pragma once

namespace Stdlib
{

const char *TruncateFileName(const char *file_name);

class Error final
{
public:
    Error();

    Error(int code);

    Error(int code, const char *subsystem, const char *message, const char *func, const char *file, int line);

    ~Error();

    int Code() const;

    void SetCode(int code);

    bool operator!= (const Error& other) const;

    bool operator== (const Error& other) const;

    void Reset();

    const char* GetFile() const;

    const char* GetFunc() const;

    int GetLine() const;

    Error(const Error& other);

    Error(Error&& other);

    Error& operator=(const Error& other);

    Error& operator=(Error&& other);

    operator bool() const;

private:
    const char* func_;
    const char* file_;
    const char *message_;
    const char *subsystem_;
    int line_;
    int code_;
};

}

#define STDLIB_ERRNO_ERROR(errno_code) \
    Stdlib::Error((errno_code), "linux", "unknown", __FUNCTION__, Stdlib::TruncateFileName(__FILE__), __LINE__)

#define STDLIB_ERROR(code, subsys, message) \
    Stdlib::Error((code), (subsys), (message), __FUNCTION__, Stdlib::TruncateFileName(__FILE__), __LINE__)
