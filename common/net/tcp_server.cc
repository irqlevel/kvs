#include "tcp_server.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <common/sync/signal.h>
#include <common/sync/process.h>
#include <common/sync/autolock.h>
#include <common/stdlib/trace.h>

namespace Net
{

static const int ServerLL = 5;

TcpServer::Connection::Connection(TcpServer& server, IO::CoEpoll& epoll, Net::Socket* socket)
    : socket_(socket)
    , epoll_(epoll)
    , server_(server)
{
}

TcpServer::Connection::~Connection()
{
}

Net::Socket* TcpServer::Connection::GetSocket()
{
    return socket_.Get();
}

IO::CoEpoll& TcpServer::Connection::GetEpoll()
{
    return epoll_;
}

TcpServer& TcpServer::Connection::GetServer()
{
    return server_;
}

Stdlib::Result<size_t, Stdlib::Error> TcpServer::Connection::Read(void* buf, size_t buf_size)
{
    return GetSocket()->Read(buf, buf_size);
}

Stdlib::Result<size_t, Stdlib::Error> TcpServer::Connection::Write(const void* buf, size_t buf_size)
{
    return GetSocket()->Write(buf, buf_size);
}

void TcpServer::ConnCoRoutine(void *arg)
{
    TcpServer::Connection *conn = static_cast<TcpServer::Connection *>(arg);
    auto co = Sync::Coroutine::Self();

    Trace(ServerLL, "here co 0x%p\n", co);

    conn->GetServer().ConnectionCoHandler(conn);

    conn->GetEpoll().Remove(co->GetSignalFd(), co);
    conn->GetEpoll().Remove(conn->GetSocket()->GetFd(), co);

    delete conn;
}

TcpServer::TcpServer()
    : address_()
    , port_(0)
    , stopping_(false)
{
}

TcpServer::Thread::Thread(TcpServer& server)
    : server_(server)
{
}

TcpServer::Thread::~Thread()
{
}

void TcpServer::Thread::DumpStats()
{
}

void* TcpServer::Thread::Worker()
{
    Stdlib::Error err;
    Sync::Coroutine *co;

    err = Sync::Coroutine::Init();
    if (err)
        goto out;

    co = Sync::Coroutine::New(&TcpServer::ListenCoRoutine, this);
    if (co == nullptr) {
        err = STDLIB_ERRNO_ERROR(ENOMEM);
        goto fail;
    }

    if (epoll_.Add(listen_socket_->GetFd(), co)) {
        Trace(ServerLL, "cant add listen sock fd\n");
        err = STDLIB_ERRNO_ERROR(ENOMEM);
        Sync::Coroutine::Put(co);
        goto fail;
    }

    err = epoll_.EventLoop(&server_.stopping_);
    if (err)
        goto fail;

    Sync::Coroutine::Deinit();
    server_.wg_.Done();
    return nullptr;

fail:
    Sync::Coroutine::Deinit();
out:
    server_.wg_.Done();
    return reinterpret_cast<void*>(err.Code());
}

void* TcpServer::Thread::Routine(Sync::Thread& t, void* ctx)
{
    return static_cast<Thread*>(ctx)->Worker();
}

void TcpServer::ListenCoRoutine(void *arg)
{
    TcpServer::Thread* thread = static_cast<TcpServer::Thread*>(arg);

    Trace(ServerLL, "here\n");

    for (;;) {
        Trace(ServerLL, "accepting\n");
        auto result = thread->listen_socket_->Accept();
        auto err = result.Error();
        if (err) {
            if (err == STDLIB_ERRNO_ERROR(EAGAIN) || err == STDLIB_ERRNO_ERROR(EINTR)) {
                Sync::Coroutine::Yield();
                continue;
            }
            Trace(0, "accept error %d\n", err.Code());
            break;
        }
        
        auto connSocket = result.MutValue();
        auto conn = new Connection(thread->GetServer(), thread->GetEpoll(), connSocket);
        if (conn == nullptr) {
            Trace(ServerLL, "can't create new conn\n");
            delete connSocket;
            Sync::Coroutine::Yield();
            continue;
        }

        auto co = Sync::Coroutine::New(&TcpServer::ConnCoRoutine, conn);
        if (co == nullptr) {
            Trace(ServerLL, "can't create conn co\n");
            delete conn;
            Sync::Coroutine::Yield();
            continue;
        }

        err = thread->GetEpoll().Add(co->GetSignalFd(), co);
        if (err) {
            Trace(ServerLL, "can't add co signal fd %d err %d\n", co->GetSignalFd(), err);
            Sync::Coroutine::Put(co);
            delete conn;
            Sync::Coroutine::Yield();
            continue;
        }

        err = thread->GetEpoll().Add(conn->GetSocket()->GetFd(), co);
        if (err) {
            Trace(ServerLL, "can't add conn socket fd %d err %d\n", conn->GetSocket()->GetFd(), err);
            thread->GetEpoll().Remove(co->GetSignalFd(), co);
            Sync::Coroutine::Put(co);
            delete conn;
            Sync::Coroutine::Yield();
            continue;
        }

        Sync::Coroutine::Yield();
    }

    thread->GetEpoll().Remove(thread->listen_socket_->GetFd(), Sync::Coroutine::Self());
}

bool TcpServer::Thread::Start()
{
    int r = epoll_.Create();
    if (r) {
        Trace(ServerLL, "epoll error %d\n", r);
        return false;
    }

    listen_socket_.Reset(new Net::Socket());
    if (listen_socket_.Get() == nullptr) {
        epoll_.Close();
        return false;
    }

    r = listen_socket_->Bind(server_.GetAddress().GetConstBuf(), server_.GetPort());
    if (r) {
        Trace(ServerLL, "bind error %d\n", r);
        listen_socket_.Reset();
        epoll_.Close();
        return false;
    }

    r = listen_socket_->Listen();
    if (r) {
        Trace(ServerLL, "listen error %d\n", r);
        listen_socket_.Reset();
        epoll_.Close();
        return false;
    }

    r = thread_.Start(&TcpServer::Thread::Routine, this);
    if (r) {
        listen_socket_.Reset();
        epoll_.Close();
        return false;
    }

    return true;
}

void* TcpServer::Thread::Join()
{
    return thread_.Join();
}

int TcpServer::Thread::Kill(int signo)
{
    return thread_.Kill(signo);
}

const Stdlib::String& TcpServer::GetAddress()
{
    return address_;
}

int TcpServer::GetPort()
{
    return port_;
}

bool TcpServer::Start(const char* address, int port, int nthreads)
{
    if (address == nullptr || strlen(address) < 1 || port < 0 || nthreads < 0)
        return false;

    Sync::AutoLock lock(lock_);
    if (stopping_)
        return false;

    port_ = port;

    if (!address_.Reset(address))
        return false;

    if (nthreads == 0)
        nthreads = Sync::Thread::GetCpuCount();
    threads_.ReserveAndUse(nthreads);
    Trace(ServerLL, "pid %d um threads %d\n", Sync::Process::Getpid(), threads_.GetSize());
    if (threads_.GetSize() == 0)
        return false;


    for (size_t i = 0; i < threads_.GetSize(); i++) {
        threads_[i] = Stdlib::MakeUnique<Thread>(*this);
        if (threads_[i].Get() == nullptr)
        {
            threads_.Clear();
            return false;
        }
    }

    for (size_t i = 0; i < threads_.GetSize(); i++) {
        wg_.Add(1);
        if (!threads_[i]->Start()) {
            wg_.Done();
            Trace(ServerLL, "can't start thread %d\n", i);
            stopping_ = true;
            for (size_t j = 0; j < i; j++)
                threads_[j]->Join();
            
            threads_.Clear();
            return false;
        }
    }

    return true;
}

TcpServer::~TcpServer()
{
}

TcpServer& TcpServer::Thread::GetServer()
{
    return server_;
}

IO::CoEpoll& TcpServer::Thread::GetEpoll()
{
    return epoll_;
}

void TcpServer::Shutdown()
{
    {
        Sync::AutoLock lock(lock_);
        stopping_ = true;

        for (size_t i = 0; i < threads_.GetSize(); i++) {
            auto& t= threads_[i];
            t->Kill(Sync::Signal::kSIGTERM);
        }
    }

    wg_.Wait();

    {
        Sync::AutoLock lock(lock_);

        for (size_t i = 0; i < threads_.GetSize(); i++) {
            auto& t= threads_[i];
            t->DumpStats();
        }

        threads_.Clear();
        address_.Clear();
    }
}

}
