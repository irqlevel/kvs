#include "path.h"

#include <errno.h>

namespace FileSys
{

    Stdlib::Result<Stdlib::String, Stdlib::Error> Path::Join(const Stdlib::String &path1, const Stdlib::String &path2)
    {
        Stdlib::Result<Stdlib::String, Stdlib::Error> result;

        if (!result.MutValue().CopyFrom(path1)) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }

        if (!result.MutValue().Append('/')) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }

        if (!result.MutValue().Append(path2)) {
            result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
            return result;
        }

        return result;
    }

    Stdlib::Result<Stdlib::String, Stdlib::Error> Path::GetExtension(const Stdlib::String &path)
    {
        int extPos = path.Rfind('.');
        if (extPos < 0)
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(STDLIB_ERRNO_ERROR(ENOENT));

        return path.Substring(extPos + 1, __SIZE_MAX__);
    }
}