#include "abort.h"

#include <stdlib.h>

namespace Stdlib
{

bool Abort()
{
    ::abort();
    return true;
}

}