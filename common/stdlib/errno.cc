#include "errno.h"
#include <errno.h>

namespace Stdlib
{

int Errno::Get()
{
    return errno;
}

void Errno::Reset(int value)
{
    errno = value;
}

}