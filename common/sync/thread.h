#pragma once


#include <common/base.h>
#include <pthread.h>
#include "atomic.h"

namespace Sync
{

class Thread
{
public:
    Thread();
    virtual ~Thread();
    using ThreadRoutinePtr = void* (*)(Thread& t, void* ctx);

    int Start(ThreadRoutinePtr routine, void* context);
    void* Join();
    int Kill(int signo);

    static size_t GetCpuCount();

    static void Sleep(int secs);

    static int Gettid();

private:
    Thread(const Thread& other) = delete;
    Thread(Thread&& other) = delete;
    Thread& operator=(const Thread& other) = delete;
    Thread& operator=(Thread&& other) = delete;

    static void* ThreadRoutineProxy(void *p);

    ThreadRoutinePtr routine_;
    void* context_;
    pthread_t thread_;
    Atomic running_;
};

}
