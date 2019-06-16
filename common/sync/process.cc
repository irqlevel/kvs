#include "process.h"
#include <unistd.h>
#include <sys/types.h>

namespace Sync
{

int Process::Getpid()
{
    return ::getpid();
}

void Process::Exit(int status)
{
    ::_exit(status);
}

}