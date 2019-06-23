#pragma once

#include <common/base.h>
#include <common/sync/thread.h>
#include <common/sync/mutex.h>
#include <common/sync/condwait.h>
#include <common/stdlib/list_entry.h>
#include <common/stdlib/intrusive_ptr.h>

#include "eventfd.h"

namespace Sync
{

struct CoroutineUContext;
class Coroutine;
class CoroutineIncRef;
class CoroutineDecRef;

using CoroutinePtr = Stdlib::IntrusivePtr<Coroutine, CoroutineIncRef, CoroutineDecRef>;

class Coroutine final
{
public:
    using CoroutineRoutine = void (*)(void* data);

    static Stdlib::Error Init();

    static void Deinit();

    static CoroutinePtr New(CoroutineRoutine routine, void* routineArg);

    void Enter();

    int Signal();

    Stdlib::Result<u64, Stdlib::Error> ReadSignal();

    int GetSignalFd();

    static void Yield();

    static CoroutinePtr Self();

    static bool InCoroutine();

    long GetRefCounter();

private:
    friend struct CoroutineUContext;
    friend class CoroutineIncRef;
    friend class CoroutineDecRef;

    Coroutine();
    ~Coroutine();

    Coroutine(const Coroutine& other) = delete;
    Coroutine(Coroutine&& other) = delete;
    Coroutine& operator=(const Coroutine& other) = delete;
    Coroutine& operator=(Coroutine&& other) = delete;

    static void Swap(Coroutine* from, Coroutine* to, int action);
    static int __attribute__((noinline)) Switch(Coroutine* from, Coroutine* to, int action);

    static Coroutine* RawSelf();

    void Terminate();

    void IncRefCounter();

    long DecRefCounter();

    static const int COROUTINE_YIELD = 1;
    static const int COROUTINE_TERMINATE = 2;
    static const int COROUTINE_ENTER = 3;

    Eventfd _Eventfd;
    CoroutineRoutine _Routine;
    void* _RoutineArg;
    void* _TrampolineArg;
    Coroutine* _Caller;
};

struct CoroutineListEntry {
    Stdlib::ListEntry list_entry_;
    CoroutinePtr co_;

    CoroutineListEntry(CoroutinePtr co)
        : co_(co)
    {
    }
};

class CoroutineIncRef
{
public:
    void operator() (Coroutine *co) {
        co->IncRefCounter();
    }
};

class CoroutineDecRef
{
public:
    void operator() (Coroutine *co) {
        co->DecRefCounter();
    }
};


size_t HashCoroutinePtr(CoroutinePtr const& value);

}
