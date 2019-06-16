#pragma once

#include <common/base.h>
#include <common/stdlib/error.h>

namespace Sync
{

class Signal final
{
public:
    using Handler = void (*)(int signo);

    static Signal& GetInstance() {
        static Signal g_signal_instance;
        return g_signal_instance;
    }

    Stdlib::Error SetHandler(int signo, Handler handler);

    static const int kSIGINT = 2;
    static const int kSIGUSR1 = 10;
    static const int kSIGPIPE = 13;
    static const int kSIGTERM = 15;

private:
    Signal();
    ~Signal();
    Signal(const Signal& other) = delete;
    Signal(Signal&& other) = delete;
    Signal& operator=(const Signal& other) = delete;
    Signal& operator=(Signal&& other) = delete;
};

}