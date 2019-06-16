#pragma once

#include "base.h"
#include "time.h"

#include <common/sync/thread.h>
#include <common/io.h>

namespace Stdlib
{

class Tracer
{
public:
    static Tracer& GetInstance()
    {
        static Tracer g_Tracer;

        return g_Tracer;
    }

    Tracer();

    void Output(const char *fmt, ...);

    void SetLevel(int level);

    int GetLevel();

    void SetOutput(IO::Writer *outputDst);

private:
    virtual ~Tracer();
    Tracer(const Tracer& other) = delete;
    Tracer(Tracer&& other) = delete;
    Tracer& operator=(const Tracer& other) = delete;
    Tracer& operator=(Tracer&& other) = delete;

    IO::Writer *OutputDst;
    int Level;
};

}

#define Trace(level, fmt, ...)                                      \
do {                                                                \
    auto& tracer = Stdlib::Tracer::GetInstance();                   \
    if (unlikely((level) <= tracer.GetLevel()))                     \
    {                                                               \
        auto now = Stdlib::Clock().Now();                          \
        tracer.Output("%lu.%06lu %d %d %s(),%s,%u " fmt,             \
            now.Seconds, now.Microseconds, (level), Sync::Thread::Gettid(),               \
            __func__, Stdlib::TruncateFileName(__FILE__),           \
            __LINE__, ##__VA_ARGS__);                        \
    }                                                               \
} while (false)


#define TraceEx(tracer, level, fmt, ...)                             \
do {                                                                \
    if (unlikely((level) <= (tracer).GetLevel()))                     \
    {                                                               \
        auto now = Stdlib::Clock().Now();                          \
        (tracer).Output("%lu.%06lu %d %d %s(),%s,%u " fmt,             \
            now.Seconds, now.Microseconds, (level), Sync::Thread::Gettid(),               \
            __func__, Stdlib::TruncateFileName(__FILE__),           \
            __LINE__, ##__VA_ARGS__);                        \
    }                                                               \
} while (false)
