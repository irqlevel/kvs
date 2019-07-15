#include "aio.h"

#include <common/stdlib/errno.h>
#include <common/stdlib/trace.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <linux/aio_abi.h>
#include <memory.h>
#include <errno.h>

namespace IO
{

    inline int io_setup(unsigned nr, aio_context_t *ctxp) {
        return syscall(__NR_io_setup, nr, ctxp);
    }

    inline int io_destroy(aio_context_t ctx) {
        return syscall(__NR_io_destroy, ctx);
    }

    inline int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) {
        return syscall(__NR_io_submit, ctx, nr, iocbpp);
    }

    inline int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
            struct io_event *events, struct timespec *timeout) {
        return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
    }

    AioContext::AioContext()
        : _ctx_id(0)
    {
    }

    AioContext::~AioContext()
    {
        if (_ctx_id)
            Destroy();
    }

    Stdlib::Error AioContext::Setup()
    {
        int r = io_setup(1, &_ctx_id);
        if (r < 0)
            return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        return 0;
    }

    Stdlib::Error AioContext::Submit(int fd, s64 offset, void *buf, size_t buf_size, int event_fd, int flags, int op)
    {
        struct iocb cb;
	    struct iocb *cbs[1];

        memset(&cb, 0, sizeof(cb));
	    cb.aio_fildes = fd;

        if (op == kRead)
            cb.aio_lio_opcode = IOCB_CMD_PREAD;
        else if (op == kWrite)
            cb.aio_lio_opcode = IOCB_CMD_PWRITE;
        else if (op == kDataSync)
            cb.aio_lio_opcode = IOCB_CMD_FDSYNC;
        else
            return STDLIB_ERRNO_ERROR(EINVAL);

        cb.aio_buf = reinterpret_cast<ulong>(buf);
        cb.aio_offset = offset;
        cb.aio_nbytes = buf_size;
        cb.aio_resfd = event_fd;

        if (flags & kEventfd) {
            cb.aio_flags = IOCB_FLAG_RESFD;
            cb.aio_resfd = event_fd;
        }

        cbs[0] = &cb;
        int r = io_submit(_ctx_id, 1, cbs);
        if (r < 0)
            return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        return 0;
    }

    Stdlib::Result<s64, Stdlib::Error> AioContext::Wait()
    {
        struct io_event events[1];

        int r = io_getevents(_ctx_id, 0, 1, events, NULL);
        if (r < 0)
            return Stdlib::Result<s64, Stdlib::Error>(STDLIB_ERRNO_ERROR(Stdlib::Errno::Get()));

        if (r == 0)
            return Stdlib::Result<s64, Stdlib::Error>(STDLIB_ERRNO_ERROR(EAGAIN));

        //Trace(0, "res %lld res2 %lld\n", events[0].res, events[0].res2); 
        return Stdlib::Result<s64, Stdlib::Error>(events[0].res, 0);
    }

    Stdlib::Error AioContext::Destroy()
    {
        int r = io_destroy(_ctx_id);
        if (r < 0)
            return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());
        return 0;
    }
}