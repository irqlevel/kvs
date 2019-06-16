#include "signal.h"
#include <signal.h>
#include <unistd.h>
#include <common/stdlib/errno.h>

namespace Sync
{

Signal::Signal()
{
}

Signal::~Signal()
{
}

Stdlib::Error Signal::SetHandler(int signo, Handler handler)
{
    if (signal(signo, handler) == SIG_ERR)
        return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

    return 0;
}

}