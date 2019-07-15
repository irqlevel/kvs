#pragma once

#include <common/stdlib/error.h>
#include <common/stdlib/result.h>
#include <common/stdlib/string.h>
#include <common/stdlib/shared_ptr.h>
#include <common/stdlib/hash_map.h>
#include <common/sync/corwmutex.h>

#include <common/filesys/file.h>

namespace Lbs
{
    class Disk
    {
    public:
        Disk();
        virtual ~Disk();

        Stdlib::Error Init(const Stdlib::String &name, const Stdlib::String &disk_id, s64 size, s64 block_size);

        Stdlib::Result<size_t, Stdlib::Error> Write(s64 offset, void *data, size_t data_size);

        Stdlib::Result<size_t, Stdlib::Error> Read(s64 offset, void *data, size_t data_size);

        Stdlib::Error Sync();

        const Stdlib::String& GetName() const;

        const Stdlib::String& GetDiskId() const;

        s64 GetSize();

        s64 GetBlockSize();

    private:
        Stdlib::Error CheckIoParameters(s64 offset, size_t data_size);

        Stdlib::String name_;
        Stdlib::String disk_id_;
        s64 size_;
        s64 block_size_;
        FileSys::File file_;
    };

    using DiskPtr = Stdlib::SharedPtr<Disk>;

    class DiskManager
    {
    public:
        DiskManager();
        virtual ~DiskManager();

        Stdlib::Result<Stdlib::String, Stdlib::Error> AddDisk(const Stdlib::String& name, s64 size, s64 block_size);

        Stdlib::Result<DiskPtr, Stdlib::Error> LookupDisk(const Stdlib::String& disk_id);

        void Shutdown();

    private:

        Stdlib::Result<Stdlib::String, Stdlib::Error> GenerateDiskId();
        Stdlib::HashMap<Stdlib::String, DiskPtr, 17, &Stdlib::String::Hash> disk_map_;
        Sync::CoRwMutex disk_map_lock_;
    };
}