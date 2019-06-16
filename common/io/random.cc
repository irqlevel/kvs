#include "random.h"

#include <sys/random.h>
#include <common/stdlib/abort.h>

namespace IO
{
    Random::Random()
    {
    }

    Random::~Random()
    {
    }

    void Random::GetRandomBytes(void *buf, size_t buf_size)
    {
        auto result = ::getrandom(buf, buf_size, 0);
        if (result < 0)
            Stdlib::Abort();

        if (static_cast<size_t>(result) != buf_size)
            Stdlib::Abort();
    }
}