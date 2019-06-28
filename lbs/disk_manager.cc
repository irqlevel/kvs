#include "disk_manager.h"
#include "error.h"

#include <common/io/random.h>
#include <common/io/aio.h>
#include <common/sync/coroutine.h>

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
        if (!_name.CopyFrom(name))
            return LbsErrNoMemory;

        if (!_disk_id.CopyFrom(disk_id))
            return LbsErrNoMemory;

        auto err = _file.Open(name, FileSys::File::kDirect|FileSys::File::kSync|FileSys::File::KReadWrite);
        if (err)
            return err;

        _size = size;
        _block_size = block_size;
        return 0;
    }

    Stdlib::Result<size_t, Stdlib::Error> Disk::Write(s64 offset, void *data, size_t data_size)
    {
        IO::AioContext aio;

        auto err = aio.Setup();
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        err = aio.Submit(_file.GetFd(), offset, data, data_size, Sync::Coroutine::Self()->GetSignalFd(), IO::AioContext::kWrite|IO::AioContext::kEventfd);
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        for (;;) {
            auto result = aio.Wait();
            if (result.Error()) {
                if (result.Error() == STDLIB_ERRNO_ERROR(EAGAIN) || result.Error() == STDLIB_ERRNO_ERROR(EINTR)) {
                    Sync::Coroutine::Yield();
                    continue;
                }
                return Stdlib::Result<size_t, Stdlib::Error>(result.Error());
            } else
                return Stdlib::Result<size_t, Stdlib::Error>(result.Value(), 0);             
        }
    }

    Stdlib::Result<size_t, Stdlib::Error> Disk::Read(s64 offset, void *data, size_t data_size)
    {
        IO::AioContext aio;

        auto err = aio.Setup();
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        err = aio.Submit(_file.GetFd(), offset, data, data_size, Sync::Coroutine::Self()->GetSignalFd(), IO::AioContext::kEventfd);
        if (err)
            return Stdlib::Result<size_t, Stdlib::Error>(err);

        for (;;) {
            auto result = aio.Wait();
            if (result.Error()) {
                if (result.Error() == STDLIB_ERRNO_ERROR(EAGAIN) || result.Error() == STDLIB_ERRNO_ERROR(EINTR)) {
                    Sync::Coroutine::Yield();
                    continue;
                }
                return Stdlib::Result<size_t, Stdlib::Error>(result.Error());
            } else
                return Stdlib::Result<size_t, Stdlib::Error>(result.Value(), 0);             
        }
    }

    const Stdlib::String& Disk::GetName() const
    {
        return _name;
    }

    const Stdlib::String& Disk::GetDiskId() const
    {
        return _disk_id;
    }

    s64 Disk::GetSize()
    {
        return _size;
    }

    s64 Disk::GetBlockSize()
    {
        return _block_size;
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

        _disk_map_lock.Lock();
        if (!_disk_map.Insert(disk->GetDiskId(), disk)) {
            _disk_map_lock.Unlock();
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(LbsErrDiskAlreadyExists);
        }
        _disk_map_lock.Unlock();

        return Stdlib::Result<Stdlib::String, Stdlib::Error>(Stdlib::Move(disk_id_result.MutValue()), 0);
    }

    Stdlib::Result<DiskPtr, Stdlib::Error> DiskManager::LookupDisk(const Stdlib::String& disk_id)
    {
        DiskPtr disk;

        _disk_map_lock.ReadLock();
        auto it = _disk_map.Lookup(disk_id);
        if (it != _disk_map.End())
            disk = *it;
        _disk_map_lock.Unlock();

        if (disk.Get() == nullptr)
            return Stdlib::Result<DiskPtr, Stdlib::Error>(LbsErrDiskNotFound);

        return Stdlib::Result<DiskPtr, Stdlib::Error>(disk, 0);
    }

    Stdlib::Result<Stdlib::String, Stdlib::Error> DiskManager::GenerateDiskId()
    {
        Stdlib::String result;
        u8 bytes[32];

        auto rng = IO::Random();
        rng.GetRandomBytes(bytes, sizeof(bytes));

        if (!result.EncodeToHex(bytes, sizeof(bytes)))
            return Stdlib::Result<Stdlib::String, Stdlib::Error>(LbsErrHexEncoding);

        return Stdlib::Result<Stdlib::String, Stdlib::Error>(Stdlib::Move(result), 0);
    }

}