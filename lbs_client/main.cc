#include <common/net/tcp_req_server.h>
#include <common/sync/process.h>
#include <common/stdlib/trace.h>
#include <common/io/random.h>
#include <common/stdlib/unique_ptr.h>
#include "client.h"

struct ClientContext {
    const char *address_;
    int port_;
    Sync::Atomic running_count_;
    IO::CoEpoll epoll_;
    bool stopping_;
};

void clientCoRoutine(void *arg)
{
    auto co = Sync::Coroutine::Self();
    auto context = static_cast<ClientContext *>(arg);

    auto client = Stdlib::MakeUnique<Lbs::Client>();
    auto err = client->Connect(context->address_, context->port_);
    if (err) {
        Trace(0, "connect error %d\n", err.Code());
        Sync::Process::Exit(1);
    }

    err = context->epoll_.Add(client->GetSocket()->GetFd(), co);
    if (err) {
        Trace(0, "can't add signal fd\n");
        Sync::Process::Exit(1);
    }

    auto result = client->AddDisk("/dev/loop10");
    if (result.Error()) {
        Trace(0, "can't add disk\n");
        Sync::Process::Exit(1);
    }
    auto &disk_id = result.Value();
    Trace(0, "disk_id %s\n", disk_id.GetConstBuf());

    Stdlib::ByteArray<u8> data;
    if (!data.ReserveAndUse(4096)) {
        Trace(0, "can't reserve memory for data\n");
        Sync::Process::Exit(1);
    }

    auto rng = IO::Random();
    rng.GetRandomBytes(data.GetBuf(), data.GetSize());

    auto result2 = client->WriteDisk(disk_id, 0, data);
    if (result2.Error()) {
        Trace(0, "can't write disk\n");
        Sync::Process::Exit(1);
    }
    Trace(0, "written %lld\n", result2.Value());

    auto result3 = client->ReadDisk(disk_id, 0, 4096);
    if (result3.Error()) {
        Trace(0, "can't read disk\n");
        Sync::Process::Exit(1);
    }
    auto& read_data = result3.Value();
    Trace(0, "read %lu\n", read_data.GetSize());

    if (read_data.GetSize() != data.GetSize() || Stdlib::MemCmp(read_data.GetConstBuf(), data.GetConstBuf(), data.GetSize()) != 0) {
        Trace(0, "data differ\n");
        Sync::Process::Exit(1);
    }

    context->epoll_.Remove(client->GetSocket()->GetFd(), co);
    context->epoll_.Remove(co->GetSignalFd(), co);

    if (context->running_count_.Dec() == 0)
        context->stopping_ = true;
}

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

    auto err = Sync::Coroutine::Init();
    if (err) {
        Trace(0, "can't init coroutines err %d\n", err.Code());
        Sync::Process::Exit(1);
    }

    {
        ClientContext context;
        context.port_ = port.Value();
        context.address_ = argv[1];

        err = context.epoll_.Create();
        if (err) {
            Trace(0, "can't create epoll err %d\n", err.Code());
            Sync::Process::Exit(1);
        }

        for (int i = 0; i < 1; i++) {
            context.running_count_.Inc();
            auto co = Sync::Coroutine::New(clientCoRoutine, &context);
            if (co == nullptr) {
                Trace(0, "can't create coroutine\n");
                Sync::Process::Exit(1);
            }
            err = context.epoll_.Add(co->GetSignalFd(), co);
            if (err) {
                Trace(0, "can't add signal fd\n");
                Sync::Process::Exit(1);
            }
        }

        context.stopping_ = false;
        err = context.epoll_.EventLoop(&context.stopping_);
    }

    Sync::Coroutine::Deinit();
    return 0;
}