#include "disk_manager.h"
#include "error.h"

#include <common/io/random.h>
#include <common/sync/coroutine.h>

namespace Lbs
{
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