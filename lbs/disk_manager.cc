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
        if (result.Error())
            return result.Error();

        auto result2 = file_.Seek(0, FileSys::File::kSeekSet);
        if (result2.Error())
            return result2.Error();

        if (size > result.Value())
            return STDLIB_ERRNO_ERROR(EINVAL);

        if (size == 0)
            size = result.Value();

        if (block_size == 0)
            block_size_ = 4096;

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
        if (err) {
            return Stdlib::Result<size_t, Stdlib::Error>(err);
        }

        Memory::Mmap mmap;
        err = mmap.Map(Stdlib::SizeInPages(data_size));
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        Stdlib::MemCpy(mmap.GetBuf(), data, data_size);
        return file_.WriteAt(offset, mmap.GetBuf(), data_size);
    }

    Stdlib::Result<size_t, Stdlib::Error> Disk::Read(s64 offset, void *data, size_t data_size)
    {
        auto err = CheckIoParameters(offset, data_size);
        if (err) {
            return Stdlib::Result<size_t, Stdlib::Error>(err);
        }

        Memory::Mmap mmap;
        err = mmap.Map(Stdlib::SizeInPages(data_size));
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        auto result = file_.ReadAt(offset, mmap.GetBuf(), data_size);
        if (result.Error())
            return Stdlib::Result<size_t, Stdlib::Error>(result.Error());

        s64 bytes_read = result.Value();
        if (bytes_read < 0)
            return Stdlib::Result<size_t, Stdlib::Error>(STDLIB_ERRNO_ERROR(bytes_read));

        Stdlib::MemCpy(data, mmap.GetBuf(), bytes_read);
        return Stdlib::Result<size_t, Stdlib::Error>(bytes_read, 0);
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

    DiskManager::DiskManager()
    {
    }

    DiskManager::~DiskManager()
    {
    }

    Stdlib::Result<Stdlib::String, Stdlib::Error> DiskManager::AddDisk(const Stdlib::String& name, s64 size, s64 block_size)
    {
        auto disk = Stdlib::MakeShared<Disk>();
        if (disk.Get() == nullptr)
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(LbsErrNoMemory);

        auto disk_id_result = GenerateDiskId();
        if (disk_id_result.Error())
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(disk_id_result.Error());

        auto err = disk->Init(name, disk_id_result.Value(), size, block_size);
        if (err)
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(err);

        disk_map_lock_.Lock();
        if (!disk_map_.Insert(disk->GetDiskId(), disk)) {
            disk_map_lock_.Unlock();
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(LbsErrDiskAlreadyExists);
        }
        disk_map_lock_.Unlock();

        Trace(0, "added disk name %s id %s size %lld\n", disk->GetName().GetConstBuf(), disk->GetDiskId().GetConstBuf(), disk->GetSize());

        return Stdlib::Result<Stdlib::String, Stdlib::Error>(Stdlib::Move(disk_id_result.MutValue()), 0);
    }

    Stdlib::Result<DiskPtr, Stdlib::Error> DiskManager::LookupDisk(const Stdlib::String& disk_id)
    {
        DiskPtr disk;

        //Trace(0, "lookup disk %s\n", disk_id.GetConstBuf());

        disk_map_lock_.ReadLock();
        auto it = disk_map_.Lookup(disk_id);
        if (it != disk_map_.End())
            disk = *it;
        disk_map_lock_.Unlock();

        if (disk.Get() == nullptr)
            return Stdlib::Result<DiskPtr, Stdlib::Error>(LbsErrDiskNotFound);

        return Stdlib::Result<DiskPtr, Stdlib::Error>(disk, 0);
    }

    Stdlib::Result<Stdlib::String, Stdlib::Error> DiskManager::GenerateDiskId()
    {
        Stdlib::String result;
        u8 bytes[8];

        auto rng = IO::Random();
        rng.GetRandomBytes(bytes, sizeof(bytes));

        if (!result.EncodeToHex(bytes, sizeof(bytes)))
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(LbsErrHexEncoding);

        return Stdlib::Result<Stdlib::String, Stdlib::Error>(Stdlib::Move(result), 0);
    }

    void DiskManager::Shutdown()
    {
        disk_map_.Clear();
    }
}