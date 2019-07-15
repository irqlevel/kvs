#include <common/net/tcp_req_server.h>
#include <common/sync/process.h>
#include <common/stdlib/trace.h>
#include <common/io/random.h>
#include <common/stdlib/unique_ptr.h>
#include "client.h"

int main(int argc, char *argv[])
{
    auto& tracer = Stdlib::Tracer::GetInstance();
    tracer.SetLevel(0);

    if (argc != 3) {
        Trace(0, "invalid count of args\n");
        Sync::Process::Exit(1);
    }

    for (int i = 0; i < argc; i++)
        Trace(0, "arg[%d]=%s\n", i, argv[i]);

    auto port = Stdlib::Str2Ulong(argv[2], 10);
    if (port.Error()) {
        Trace(0, "decode port err %d\n", port.Error().Code());
        Sync::Process::Exit(1);
    }

    s64 block_size = 4096;
    auto client = Stdlib::MakeUnique<Lbs::Client>();
    auto err = client->Connect(argv[1], port.Value(), false);
    if (err) {
        Trace(0, "connect error %d\n", err.Code());
        Sync::Process::Exit(1);
    }

    Trace(0, "adding disk\n");

    auto diskId = client->AddDisk("/dev/loop10", block_size);
    if (diskId.Error()) {
        Trace(0, "can't add disk\n");
        Sync::Process::Exit(1);
    }

    Trace(0, "generating data\n");

    Stdlib::ByteArray<u8> data;
    if (!data.ReserveAndUse(block_size)) {
        Trace(0, "can't reserve memory for data\n");
        Sync::Process::Exit(1);
    }

    auto rng = IO::Random();
    rng.GetRandomBytes(data.GetBuf(), data.GetSize());

    Trace(0, "writting to disk\n");

    for (int i = 0; i < 100000; i++) {
        auto result2 = client->WriteDisk(diskId.Value(), i * block_size, data);
        if (result2.Error()) {
            Trace(0, "can't write disk\n");
            Sync::Process::Exit(1);
        }
    }

    Trace(0, "syncing\n");
    err = client->SyncDisk(diskId.Value());
    if (err) {
        Trace(0, "can't sync disk\n");
        Sync::Process::Exit(1);
    }

    Trace(0, "fin\n");
    return 0;
}