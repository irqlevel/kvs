#include "abort.h"
#include "trace.h"

#include <stdlib.h>

namespace Stdlib
{

bool Abort()
{
    ::abort();
    return true;
}

bool Abort(const char *file, int line, const char *func)
{
    Trace(0, "abort at %s,%d %s()\n", file, line, func);
    return Abort();
}

}