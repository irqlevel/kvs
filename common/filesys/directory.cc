#include "directory.h"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <common/stdlib/errno.h>

namespace FileSys
{
    Directory::Directory()
        : dir_handle_(nullptr)
    {
    }

    Directory::~Directory()
    {
        Close();
    }

    void Directory::Close()
    {
        if (dir_handle_ != nullptr) {
            ::closedir(static_cast<DIR*>(dir_handle_));
            dir_handle_ = nullptr;
        }
    }

    Stdlib::Error Directory::Open(const char *path)
    {
        Stdlib::String lpath;
        if (!lpath.Reset(path))
            return STDLIB_ERRNO_ERROR(ENOMEM);

        return Open(lpath);
    }

    Stdlib::Error Directory::Open(const Stdlib::String &path)
    {
        if (dir_handle_ != nullptr)
            return STDLIB_ERRNO_ERROR(EBADF);

        if (!path_.Reset(path))
            return STDLIB_ERRNO_ERROR(ENOMEM);
        
        dir_handle_ = ::opendir(path_.GetConstBuf());
        if (dir_handle_ == nullptr)
            return STDLIB_ERRNO_ERROR(Stdlib::Errno::Get());

        return 0;
    }

    Stdlib::Result<Stdlib::Vector<DirectoryRecord>, Stdlib::Error> Directory::List()
    {
        Stdlib::Result<Stdlib::Vector<DirectoryRecord>, Stdlib::Error> result;

        if (dir_handle_ == nullptr) {
            result.SetError(STDLIB_ERRNO_ERROR(EBADF));
            return result;
        }

        ::rewinddir(static_cast<DIR*>(dir_handle_));

        for (;;) {
            Stdlib::Errno::Reset(0);
            auto dirEntry = ::readdir(static_cast<DIR*>(dir_handle_));
            if (dirEntry == nullptr) {
                int err = Stdlib::Errno::Get();
                if (err) {
                    result.SetError(STDLIB_ERRNO_ERROR(err));
                    return result;
                }
                break;
            }

            DirectoryRecord dirRecord;
            if (!dirRecord.file_name_.Reset(dirEntry->d_name)) {
                result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
                return result;
            }

            switch (dirEntry->d_type)
            {
            case DT_REG:
                dirRecord.type_ = DirectoryRecord::kRegular;
                break;
            case DT_DIR:
                dirRecord.type_ = DirectoryRecord::kDirectory;
                break;
            default:
                dirRecord.type_ = DirectoryRecord::kUnknown;
                break;
            }

            if (!result.MutValue().PushBack(Stdlib::Move(dirRecord))) {
                result.SetError(STDLIB_ERRNO_ERROR(ENOMEM));
                return result;
            }
        }

        return result;
    }

    const Stdlib::String& Directory::GetPath() const
    {
        return path_;
    }
}