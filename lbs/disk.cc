#include "disk_manager.h"
#include "error.h"

#include <common/io/random.h>
#include <common/io/aio.h>
#include <common/sync/coroutine.h>
#include <common/malloc/mmap.h>

namespace Lbs
{
    Disk::Disk()
    {
    }

    Disk::~Disk()
    {
    }

    ulong Disk::GetBitmapBlockCount()
    {
        return Stdlib::RoundUp(Stdlib::RoundUp(bitmap_.GetSize(), block_size_)/block_size_, 8);
    }

    Stdlib::Error Disk::Init(const Stdlib::String &name, const Stdlib::String &disk_id, s64 size, s64 block_size)
    {
        if (!name_.CopyFrom(name))
            return LbsErrNoMemory;

        if (!disk_id_.CopyFrom(disk_id))
            return LbsErrNoMemory;

        auto err = file_.Open(name, FileSys::File::kReadWrite);
        if (err) {
            //Trace(0, "can't open file %s\n", name.GetConstBuf());
            return err;
        }

        auto result = file_.Seek(0, FileSys::File::kSeekEnd);
        if (result.Error()) {
            file_.Close();
            return result.Error();
        }

        auto result2 = file_.Seek(0, FileSys::File::kSeekSet);
        if (result2.Error()) {
            file_.Close();
            return result2.Error();
        }

        if (size > result.Value()) {
            file_.Close();
            return STDLIB_ERRNO_ERROR(EINVAL);
        }

        if (size == 0)
            size = result.Value();

        if (block_size == 0)
            block_size = kPageSize;
        else if (block_size % kPageSize) {
            file_.Close();
            return STDLIB_ERRNO_ERROR(EINVAL);
        }

        if (size % block_size) {
            file_.Close();
            return STDLIB_ERRNO_ERROR(EINVAL);
        }

        if (!bitmap_.Alloc(size / block_size)) {
            file_.Close();
            return STDLIB_ERRNO_ERROR(ENOMEM);
        }

        auto result3 = file_.ReadAt(0, bitmap_.GetBuf(), bitmap_.GetSize());
        if (result3.Error()) {
            file_.Close();
            return result3.Error();
        }

        if (result3.Value() != bitmap_.GetSize()) {
            file_.Close();
            return STDLIB_ERRNO_ERROR(EIO);
        }

        for (ulong i = 0; i < GetBitmapBlockCount(); i++)
            bitmap_.SetBit(i);

        result3 = file_.WriteAt(0, bitmap_.GetBuf(), GetBitmapBlockCount()/8);
        if (result3.Error()) {
            file_.Close();
            return result3.Error();
        }

        if (result3.Value() != (GetBitmapBlockCount()/8)) {
            file_.Close();
            return STDLIB_ERRNO_ERROR(EIO);
        }

        err = file_.Sync();
        if (err) {
            file_.Close();
            return err;
        }

        size_ = size;
        block_size_ = block_size;
        return 0;
    }

    Stdlib::Error Disk::CheckIoParameters(s64 offset, size_t data_size)
    {
        if (size_ < static_cast<s64>(data_size))
            return STDLIB_ERRNO_ERROR(EINVAL);

        if (offset > (size_ - static_cast<s64>(data_size)))
            return STDLIB_ERRNO_ERROR(EINVAL);

        if (offset % block_size_ != 0)
            return STDLIB_ERRNO_ERROR(EINVAL);

        if (data_size % block_size_ != 0)
            return STDLIB_ERRNO_ERROR(EINVAL);

        return 0;
    }
/*
    Stdlib::Result<size_t, Stdlib::Error> Disk::Write(s64 offset, void *data, size_t data_size)
    {
        IO::AioContext aio;
        Memory::Mmap mmap;

        auto err = CheckIoParameters(offset, data_size);
        if (err) {
            return Stdlib::Result<size_t, Stdlib::Error>(err);
        }

        err = aio.Setup();
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        err = mmap.Map(Stdlib::SizeInPages(data_size));
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        Stdlib::MemCpy(mmap.GetBuf(), data, data_size);

        err = aio.Submit(file_.GetFd(), offset, mmap.GetBuf(), data_size, Sync::Coroutine::Self()->GetSignalFd(), IO::AioContext::kWrite|IO::AioContext::kEventfd);
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        for (;;) {
            auto result = aio.Wait();
            if (result.Error()) {
                if (result.Error() == STDLIB_ERRNO_ERROR(EAGAIN) || result.Error() == STDLIB_ERRNO_ERROR(EINTR)) {
                    //Trace(0, "yield\n");
                    Sync::Coroutine::Yield();
                    continue;
                }
                return Stdlib::Result<size_t, Stdlib::Error>(result.Error());
            } else {
                s64 bytes_written = result.Value();
                if (bytes_written < 0)
                    return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(bytes_written));

                return Stdlib::Result<size_t, Stdlib::Error>(bytes_written, 0);
            }           
        }
    }

    Stdlib::Result<size_t, Stdlib::Error> Disk::Read(s64 offset, void *data, size_t data_size)
    {
        IO::AioContext aio;
        Memory::Mmap mmap;

        auto err = CheckIoParameters(offset, data_size);
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        err = aio.Setup();
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        err = mmap.Map(Stdlib::SizeInPages(data_size));
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        err = aio.Submit(file_.GetFd(), offset, mmap.GetBuf(), data_size, Sync::Coroutine::Self()->GetSignalFd(), IO::AioContext::kEventfd);
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        for (;;) {
            auto result = aio.Wait();
            if (result.Error()) {
                if (result.Error() == STDLIB_ERRNO_ERROR(EAGAIN) || result.Error() == STDLIB_ERRNO_ERROR(EINTR)) {
                    //Trace(0, "yield\n");
                    Sync::Coroutine::Yield();
                    continue;
                }
                return Stdlib::Result<size_t, Stdlib::Error>(result.Error());
            } else {
                s64 bytes_read = result.Value();
                if (bytes_read < 0)
                    return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(bytes_read));

                Stdlib::MemCpy(data, mmap.GetBuf(), bytes_read);
                return Stdlib::Result<size_t, Stdlib::Error>(bytes_read, 0);
            }         
        }
    }
*/
    Stdlib::Result<size_t, Stdlib::Error> Disk::Write(s64 offset, void *data, size_t data_size)
    {
        auto err = CheckIoParameters(offset, data_size);
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        for (ulong block_index = offset/block_size_; block_index < (offset/block_size_ + data_size/block_size_); block_index++) {
            if (!bitmap_.TestBit(block_index)) {

                return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(EFAULT));
            } else if (block_index < GetBitmapBlockCount()) {
                return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(EACCES));
            }
        }

        size_t total_bytes_written = 0;
        ulong block_seqno = 0;
        for (ulong block_index = offset/block_size_; block_index < (offset/block_size_ + data_size/block_size_); block_index++, block_seqno++) {
            BUG_ON(!bitmap_.TestBit(block_index));
            BUG_ON(block_index < GetBitmapBlockCount());

            Memory::Mmap mmap;
            err = mmap.Map(block_size_);
            if (err)
                return Stdlib::Result<size_t, Stdlib::Error>(err);

            Stdlib::MemCpy(mmap.GetBuf(), Stdlib::MemAdd(data, block_seqno * block_size_), block_size_);

            auto result = file_.WriteAt(block_index * block_size_, mmap.GetBuf(), block_size_);
            if (result.Error())
                return Stdlib::Result<size_t, Stdlib::Error>(result.Error());

            s64 bytes_written = result.Value();
            if (bytes_written < 0)
                return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(bytes_written));
            if (bytes_written != block_size_)
                return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(EIO));

            total_bytes_written += block_size_;
        }

        return Stdlib::Result<size_t, Stdlib::Error>(total_bytes_written, 0);
    }

    Stdlib::Result<size_t, Stdlib::Error> Disk::Read(s64 offset, void *data, size_t data_size)
    {
        auto err = CheckIoParameters(offset, data_size);
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        size_t total_bytes_read = 0;
        ulong block_seqno = 0;
        for (ulong block_index = offset/block_size_; block_index < (offset/block_size_ + data_size/block_size_); block_index++, block_seqno++) {
            if (!bitmap_.TestBit(block_index)) {

                Stdlib::MemSet(Stdlib::MemAdd(data, block_index * block_size_), 0, block_size_);
            } else {
                Memory::Mmap mmap;
                err = mmap.Map(block_size_);
                if (err)
                    return Stdlib::Result<size_t, Stdlib::Error>(err);

                auto result = file_.ReadAt(block_index * block_size_, mmap.GetBuf(), block_size_);
                if (result.Error())
                    return Stdlib::Result<size_t, Stdlib::Error>(result.Error());

                s64 bytes_read = result.Value();
                if (bytes_read < 0)
                    return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(bytes_read));
                if (bytes_read != block_size_)
                    return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(EIO));

                Stdlib::MemCpy(Stdlib::MemAdd(data, block_seqno * block_size_), mmap.GetBuf(), block_size_);
            }

            total_bytes_read += block_size_;
        }

        return Stdlib::Result<size_t, Stdlib::Error>(total_bytes_read, 0);
    }

    Stdlib::Error Disk::Sync()
    {
        IO::AioContext aio;

        auto err = aio.Setup();
        if (err)
            return err;

        err = aio.Submit(file_.GetFd(), 0, nullptr, 0, Sync::Coroutine::Self()->GetSignalFd(), IO::AioContext::kEventfd, IO::AioContext::kDataSync);
        if (err)
            return err;

        for (;;) {
            auto result = aio.Wait();
            if (result.Error()) {
                if (result.Error() == STDLIB_ERRNO_ERROR(EAGAIN) || result.Error() == STDLIB_ERRNO_ERROR(EINTR)) {
                    //Trace(0, "yield\n");
                    Sync::Coroutine::Yield();
                    continue;
                }
                return result.Error();
            }

            return 0;         
        }
    }

    const Stdlib::String& Disk::GetName() const
    {
        return name_;
    }

    const Stdlib::String& Disk::GetDiskId() const
    {
        return disk_id_;
    }

    s64 Disk::GetSize()
    {
        return size_;
    }

    s64 Disk::GetBlockSize()
    {
        return block_size_;
    }

}