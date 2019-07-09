#include "thread.h"
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>

namespace Sync
{


Thread::Thread()
    : routine_(nullptr)
    , context_(nullptr)
{
}

Thread::~Thread()
{
    Join();
}

void* Thread::ThreadRoutineProxy(void *p)
{
    Thread *t = static_cast<Thread*>(p);
    return t->routine_(*t, t->context_);
}

int Thread::Start(ThreadRoutinePtr routine, void* context)
{
    routine_ = routine;
    context_ = context;
    int r = pthread_create(&thread_, nullptr, &Thread::ThreadRoutineProxy, this);
    if (r == 0)
        running_.Set(1);
    return r;
}

void* Thread::Join()
{
    void* result = nullptr;

    if (running_.CmpXchg(1, 0) == 1) {
        auto ret = pthread_join(thread_, &result);
        if (ret)
            BUG_ON(1);
    }

    return result;
}

int Thread::Kill(int signo)
{
    return pthread_kill(thread_, signo);
}

void Thread::Sleep(int secs)
{
    sleep(secs);
}

size_t Thread::GetCpuCount()
{
    return (size_t)get_nprocs();
}

int Thread::Gettid()
{
    return syscall(SYS_gettid);
}

}
