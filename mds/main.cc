#include "server.h"

#include <common/sync/process.h>
#include <common/sync/signal.h>
#include <common/stdlib/trace.h>
#include <common/stdlib/utf8.h>

int main(int argc, char *argv[])
{
    auto& tracer = Stdlib::Tracer::GetInstance();
    tracer.SetLevel(0);

    Trace(0, "Version: %s %s\n", __DATE__, __TIME__);

    if (argc != 3) {
        Trace(0, "invalid count of args\n");
        return -1;
    }

    for (int i = 0; i < argc; i++)
        Trace(0, "arg[%d]=%s\n", i, argv[i]);

    auto& server = Mds::Server::GetInstance();

    if (Sync::Signal::GetInstance().SetHandler(Sync::Signal::kSIGTERM, Mds::Server::OnStopSignal)) {
        Trace(0, "can't setup signal handler\n");
        return -1;
    }

    if (Sync::Signal::GetInstance().SetHandler(Sync::Signal::kSIGINT, Mds::Server::OnStopSignal)) {
        Trace(0, "can't setup signal handler\n");
        return -1;
    }

    if (Sync::Signal::GetInstance().SetHandler(Sync::Signal::kSIGPIPE, Mds::Server::OnSigPipe)) {
        Trace(0, "can't setup signal handler\n");
        return -1;
    }

    Trace(0, "starting server\n");
    auto port = Stdlib::Str2Ulong(argv[2], 10);
    if (port.Error()) {
        Trace(0, "decode port err %d\n", port.Error().Code());
        return -1;
    }

    if (!server.Start(argv[1], port.Value(), 0)) {
        Trace(0, "can't run server\n");
        return -1;
    }

    Trace(0, "running\n");
    for (;;) {
        Sync::Thread::Sleep(1);
    }
    Trace(0, "exiting\n");
    return 0;
}