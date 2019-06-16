#pragma once

#include <common/base.h>
#include <common/sync/thread.h>
#include <common/sync/mutex.h>
#include <common/sync/condwait.h>
#include <common/stdlib/list_entry.h>

#include "eventfd.h"

namespace Sync
{

struct CoroutineUContext;

class Coroutine final
{
public:
    using CoroutineRoutine = void (*)(void* data);

    static Stdlib::Error Init();

    static void Deinit();

    static Coroutine* New(CoroutineRoutine routine, void* routineArg);

    void Enter();

    int Signal();

    Stdlib::Result<u64, Stdlib::Error> ReadSignal();

    int GetSignalFd();

    static void Yield();

    static void Get(Coroutine* co);

    static void Put(Coroutine* co);

    static Coroutine* Self();

    static void RunAll();

    static bool InCoroutine();

    Stdlib::ListEntry WaitListEntry;

private:
    friend struct CoroutineUContext;

    Coroutine();
    ~Coroutine();

    Coroutine(const Coroutine& other) = delete;
    Coroutine(Coroutine&& other) = delete;
    Coroutine& operator=(const Coroutine& other) = delete;
    Coroutine& operator=(Coroutine&& other) = delete;

    static void Swap(Coroutine* from, Coroutine* to, int action);
    static int __attribute__((noinline)) Switch(Coroutine* from, Coroutine* to, int action);

    void Terminate();

    static const int COROUTINE_YIELD = 1;
    static const int COROUTINE_TERMINATE = 2;
    static const int COROUTINE_ENTER = 3;

    Eventfd _Eventfd;
    CoroutineRoutine _Routine;
    void* _RoutineArg;
    void* _TrampolineArg;
    Coroutine* _Caller;
};

size_t HashCoroutinePtr(Coroutine* const& value);

}
