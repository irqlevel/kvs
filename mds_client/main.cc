#include <common/net/tcp_req_server.h>
#include <common/sync/process.h>
#include <common/stdlib/trace.h>
#include <common/io/random.h>
#include <mds/server.h>

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

    auto client = Stdlib::MakeUnique<Net::TcpReqServer::Client>();
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

    Stdlib::ByteArray<u8> request;

    auto rng = IO::Random();

    size_t buf_size;
    rng.GetRandomBytes(&buf_size, sizeof(buf_size));
    buf_size = 10000 + buf_size % 30000;

    if (!request.ReserveAndUse(buf_size)) {
        Trace(0, "no memory\n");
        Sync::Process::Exit(1);
    }
    rng.GetRandomBytes(request.GetBuf(), request.GetSize());

    auto result = client->SendRequest(Mds::Server::kEchoRequestType, request);
    if (result.Error()) {
        Trace(0, "send req error %d\n", result.Error().Code());
        Sync::Process::Exit(1);
    }

    auto& response = result.Value();

    if (request.GetSize() != response.GetSize()) {
        Trace(0, "invalid response size\n");
        Sync::Process::Exit(1);
    }

    if (Stdlib::MemCmp(request.GetConstBuf(), response.GetConstBuf(), request.GetSize()) != 0) {
        Trace(0, "invalid response content\n");
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

        for (int i = 0; i < 10000; i++) {
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