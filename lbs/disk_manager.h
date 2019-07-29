#pragma once

#include <common/stdlib/error.h>
#include <common/stdlib/result.h>
#include <common/stdlib/string.h>
#include <common/stdlib/shared_ptr.h>
#include <common/stdlib/hash_map.h>
#include <common/sync/corwmutex.h>

#include "disk.h"

namespace Lbs
{
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