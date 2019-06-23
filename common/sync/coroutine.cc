#undef _FORTIFY_SOURCE
#include "coroutine.h"
#include "atomic.h"

#include <common/malloc/malloc.h>
#include <common/stdlib/trace.h>
#include <common/stdlib/hash_set.h>
#include <common/stdlib/hash.h>

#include <assert.h>
#include <setjmp.h>
#include <memory.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>

namespace Sync
{

static const int CoLL = 10;

union CcArg {
    void *p;
    int i[2];
};

struct CoroutineUContext final
{
    static const int Magic1 = 0xCBDECBDE;

    CoroutineUContext()
        : Stack(nullptr), StackSize(0), Magic(Magic1)
    {
        memset(&Env, 0, sizeof(Env));
        StackSize = 4 * kPageSize;
        Stack = AllocStack(&StackSize);
        RefCount.Set(1);
    }

    ~CoroutineUContext()
    {
        if (Magic != Magic1)
            abort();

        Magic = 0;
        if (Stack != nullptr)
            FreeStack(Stack, StackSize);
    }

    static void* AllocStack(size_t* stackSize)
    {
        void* stack, *guardpage;
        if (*stackSize == 0 || (*stackSize % kPageSize) != 0)
            abort();

        *stackSize += kPageSize; // one extra guard page
        stack = mmap(NULL, *stackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (stack == MAP_FAILED)
            abort();

        guardpage = stack;
        if (mprotect(guardpage, 4096, PROT_NONE))
            abort();
        return stack;
    }

    static void FreeStack(void *stack, size_t stackSize)
    {
        munmap(stack, stackSize);
    }

    static void Trampoline(int i0, int i1);

    Coroutine Base;
    void* Stack;
    size_t StackSize;
    sigjmp_buf Env;
    Atomic RefCount;
    int Magic;
};

size_t HashCoroutinePtr(CoroutinePtr const& value)
{
    return Stdlib::HashPtr(value.Get());
}

size_t HashCoroutinePtr(Coroutine* const& value)
{
    return Stdlib::HashPtr(value);
}

static __thread CoroutineUContext* Leader;
static __thread Coroutine* Current;

Coroutine::Coroutine()
    : _Routine(nullptr)
    , _RoutineArg(nullptr)
    , _Caller()
{
    Trace(CoLL, "co %p ctor\n", this);
}

void CoroutineUContext::Trampoline(int i0, int i1)
{
    CcArg arg;
    CoroutineUContext *self;
    Coroutine *co;

    arg.i[0] = i0;
    arg.i[1] = i1;
    self = static_cast<CoroutineUContext*>(arg.p);
    co = &self->Base;

    /* Initialize longjmp environment and switch back the caller */
    if (!sigsetjmp(self->Env, 0)) {
        siglongjmp(*static_cast<sigjmp_buf *>(co->_TrampolineArg), 1);
    }

    for (;;) {
        co->_Routine(co->_RoutineArg);
        Coroutine::Switch(co, co->_Caller, Coroutine::COROUTINE_TERMINATE);
    }
}

CoroutinePtr Coroutine::New(CoroutineRoutine routine, void* routineArg)
{
    CoroutineUContext* ctx = new CoroutineUContext();
    if (ctx == nullptr)
        return CoroutinePtr();

    if (ctx->Stack == nullptr) {
        delete ctx;
        return CoroutinePtr();
    }

    Coroutine* co = &ctx->Base;
    int err = co->_Eventfd.Create(0, Eventfd::kNONBLOCK|Eventfd::kCLOEXEC);
    if (err) {
        delete ctx;
        return CoroutinePtr();
    }

    co->_Routine = routine;
    co->_RoutineArg = routineArg;
    co->_Caller = nullptr;

    ucontext_t oldUc, uc;
    sigjmp_buf oldEnv;
    union CcArg arg = {0};

    if (getcontext(&uc) == -1) {
        delete ctx;
        return CoroutinePtr();
    }

    co->_TrampolineArg = &oldEnv;

    uc.uc_link = &oldUc;
    uc.uc_stack.ss_sp = ctx->Stack;
    uc.uc_stack.ss_size = ctx->StackSize;
    uc.uc_stack.ss_flags = 0;

    arg.p = ctx;

    makecontext(&uc, (void (*)(void))&CoroutineUContext::Trampoline,
                2, arg.i[0], arg.i[1]);

    /* swapcontext() in, siglongjmp() back out */
    if (!sigsetjmp(oldEnv, 0)) {
        swapcontext(&oldUc, &uc);
    }

    auto result = CoroutinePtr(co);
    co->DecRefCounter();
    return result;
}

Coroutine::~Coroutine()
{
    Trace(CoLL, "co %p dtor\n", this);
}

void Coroutine::IncRefCounter()
{
    CoroutineUContext* ctx = CONTAINING_RECORD(this, CoroutineUContext, Base);
    if (ctx->Magic != CoroutineUContext::Magic1)
        abort();

    ctx->RefCount.Inc();

    Trace(CoLL, "co %p ref %ld\n", this, ctx->RefCount.Get());
}

long Coroutine::DecRefCounter()
{
    CoroutineUContext* ctx = CONTAINING_RECORD(this, CoroutineUContext, Base);
    if (ctx->Magic != CoroutineUContext::Magic1)
        abort();

    long result = ctx->RefCount.Dec();

    Trace(CoLL, "co %p ref %ld\n", this, ctx->RefCount.Get());

    if (result == 0) {
        delete ctx;
    }

    return result;
}

long Coroutine::GetRefCounter()
{
    CoroutineUContext* ctx = CONTAINING_RECORD(this, CoroutineUContext, Base);
    if (ctx->Magic != CoroutineUContext::Magic1)
        abort();

    return ctx->RefCount.Get();
}

void Coroutine::Enter()
{
    assert(_Caller == nullptr); //coroutine re-entered recursively
    Coroutine* self = Self().Get();
    _Caller = self;
    Trace(CoLL, "co enter this %p caller %p\n", this, _Caller);
    Swap(self, this, COROUTINE_ENTER);
}

int Coroutine::Switch(Coroutine* coFrom, Coroutine* coTo, int action)
{
    Trace(CoLL, "co switch %p -> %p\n", coFrom, coTo);

    CoroutineUContext* ctxFrom = CONTAINING_RECORD(coFrom, CoroutineUContext, Base);
    if (ctxFrom->Magic != CoroutineUContext::Magic1)
        abort();

    CoroutineUContext* ctxTo = CONTAINING_RECORD(coTo, CoroutineUContext, Base);
    if (ctxTo->Magic != CoroutineUContext::Magic1)
        abort();

    Current = coTo;
    int ret = sigsetjmp(ctxFrom->Env, 0);
    if (ret == 0) {
        siglongjmp(ctxTo->Env, action);
    }

    return ret;
}

void Coroutine::Swap(Coroutine* from, Coroutine* to, int action)
{
    int ret = Switch(from, to, COROUTINE_YIELD);
    switch (ret)
    {
    case COROUTINE_YIELD:
        return;
    case COROUTINE_TERMINATE:
        return;
    default:
        abort();
    }
}

void Coroutine::Yield()
{
    if (!InCoroutine())
        abort();

    auto self = Self().Get();
    Coroutine* to = self->_Caller;
    Trace(CoLL, "co yield self %p to %p\n", self, to);
    assert(to != nullptr); //coroutine is yielding to no one
    self->_Caller = nullptr;
    Swap(self, to, COROUTINE_YIELD);
}

Stdlib::Error Coroutine::Init()
{
    Trace(CoLL, "co init\n");

    Leader = new CoroutineUContext();
    if (Leader == nullptr) {
        return STDLIB_ERRNO_ERROR(ENOMEM);
    }
    if (Leader->Stack == nullptr) {
        Leader->Base.DecRefCounter();
        return STDLIB_ERRNO_ERROR(ENOMEM);
    }
    Trace(CoLL, "co init leader %p\n", &Leader->Base);

    Current = nullptr;
    return 0;
}

void Coroutine::Deinit()
{
    Trace(CoLL, "co deinit leader %p\n", &Leader->Base);
    Leader->Base.DecRefCounter();
    Current = nullptr;

    Trace(CoLL, "co deinit\n");
}

Coroutine* Coroutine::RawSelf()
{
    if (Current == nullptr)
        Current = &Leader->Base;

    return Current;
}

CoroutinePtr Coroutine::Self()
{
    return RawSelf();
}

bool Coroutine::InCoroutine()
{
    return (Current != nullptr) && (Current->_Caller != nullptr);
}

int Coroutine::Signal()
{
    int err = _Eventfd.Write(1);
    if (err)
        Trace(CoLL, "co signal error %d\n", err);
    return err;
}

int Coroutine::GetSignalFd()
{
    return _Eventfd.GetFd();
}

Stdlib::Result<u64, Stdlib::Error> Coroutine::ReadSignal()
{
    auto result = _Eventfd.Read();
    if (result.Error())
        Trace(CoLL, "co signal read error %d\n", result.Error().Code());

    return result;
}

}
