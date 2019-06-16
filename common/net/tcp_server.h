#pragma once

#include <common/base.h>
#include <common/io.h>
#include <common/sync/coroutine.h>
#include <common/sync/thread.h>
#include <common/sync/rwmutex.h>
#include <common/sync/waitgroup.h>
#include <common/net/socket.h>
#include <common/io/coepoll.h>

#include <common/stdlib/shared_ptr.h>
#include <common/stdlib/unique_ptr.h>
#include <common/stdlib/vector.h>
#include <common/stdlib/bytearray.h>
#include <common/stdlib/string.h>
#include <common/stdlib/hash_map.h>
#include <common/stdlib/hash.h>

namespace Net
{

class TcpServer
{
public:
    class Connection : public IO::ReadWriter
    {
    public:
        virtual Stdlib::Result<size_t, Stdlib::Error> Read(void* buf, size_t buf_size) override;
        virtual Stdlib::Result<size_t, Stdlib::Error> Write(const void* buf, size_t buf_size) override;

        Stdlib::Result<size_t, Stdlib::Error> ReadAtLeast(void* buf, size_t buf_size);
        Stdlib::Result<size_t, Stdlib::Error> WriteAtLeast(const void* buf, size_t buf_size);

        IO::CoEpoll& GetEpoll();
        TcpServer& GetServer();

    private:
        Connection(const Connection& other) = delete;
        Connection(Connection&& other) = delete;
        Connection& operator=(const Connection& other) = delete;
        Connection& operator=(Connection&& other) = delete;

        Connection(TcpServer& server, IO::CoEpoll& epoll, Net::Socket* socket);
        virtual ~Connection();

        Net::Socket* GetSocket();

        Stdlib::UniquePtr<Net::Socket> socket_;
        IO::CoEpoll& epoll_;
        TcpServer& server_;

        friend class Thread;
        friend class TcpServer;
    };

    TcpServer();

    bool Start(const char* address, int port, int nthreads);

    virtual ~TcpServer();

    virtual void ConnectionCoHandler(Connection* conn) = 0;

    const Stdlib::String& GetAddress();
    int GetPort();

    void Shutdown();

private:
    class Thread {
    public:
        Thread(TcpServer& server);
        virtual ~Thread();

        TcpServer& GetServer();
        IO::CoEpoll& GetEpoll();
        void DumpStats();

    private:
        Thread(const Thread& other) = delete;
        Thread(Thread&& other) = delete;
        Thread& operator=(const Thread& other) = delete;
        Thread& operator=(Thread&& other) = delete;

        bool Start();
        void* Join();
        int Kill(int signo);

        static void* Routine(Sync::Thread& t, void* ctx);
        void* Worker();

        Stdlib::UniquePtr<Net::Socket> listen_socket_;
        TcpServer& server_;
        Sync::Thread thread_;
        IO::CoEpoll epoll_;

        friend class TcpServer;
    };

    TcpServer(const TcpServer& other) = delete;
    TcpServer(TcpServer&& other) = delete;
    TcpServer& operator=(const TcpServer& other) = delete;
    TcpServer& operator=(TcpServer&& other) = delete;

    static void ConnCoRoutine(void *arg);
    static void ListenCoRoutine(void *arg);

    Stdlib::Vector<Stdlib::UniquePtr<Thread>> threads_;
    Stdlib::String address_;
    int port_;
    bool stopping_;
    Sync::WaitGroup wg_;
    Sync::RWMutex lock_;
};

}
