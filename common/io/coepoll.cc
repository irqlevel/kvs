#include "coepoll.h"

#include <common/stdlib/trace.h>
#include <common/stdlib/bytearray.h>

#include <errno.h>

namespace IO
{

    static const int CoEpollLL = 20;

    CoEpoll::CoEpoll()
        : stopping_(false)
    {
        Trace(CoEpollLL, "epoll %p ctor\n", this);
    }

    CoEpoll::~CoEpoll()
    {
        Close();
        Trace(CoEpollLL, "epoll %p dtor\n", this);
    }

    Stdlib::Error CoEpoll::Add(int fd, Sync::CoroutinePtr co)
    {
        auto it = fd_co_map_.Lookup(fd);
        if (it != fd_co_map_.End()) {
            auto coset = *it;
            if (!coset->Add(co))
                return STDLIB_ERRNO_ERROR(EEXIST);
        } else {
            auto coset = Stdlib::MakeShared<CoSetType>();
            if (coset.Get() == nullptr)
                return STDLIB_ERRNO_ERROR(ENOMEM);
            if (!coset->Add(co))
                return STDLIB_ERRNO_ERROR(ENOMEM);

            if (!fd_co_map_.Insert(Stdlib::Move(fd), Stdlib::Move(coset))) {
                return STDLIB_ERRNO_ERROR(ENOMEM);
            }

            auto err = epoll_.Add(fd, IO::Epoll::kIN | IO::Epoll::kOUT | IO::Epoll::kONESHOT);
            if (err) {
                fd_co_map_.Remove(fd);
                return err;
            }
        }
        return 0;
    }

    Stdlib::Error CoEpoll::Remove(int fd, Sync::CoroutinePtr co)
    {
        auto it = fd_co_map_.Lookup(fd);
        if (it != fd_co_map_.End()) {
            auto coset = *it;
            if (!coset->Remove(co))
                return STDLIB_ERRNO_ERROR(ENOENT);

            if (coset->GetCount() == 0) {
                fd_co_map_.Remove(fd);
                epoll_.Del(fd);
            }
            return 0;
        } else
            return STDLIB_ERRNO_ERROR(ENOENT);
    }

    Stdlib::Error CoEpoll::EventLoop(bool *stopping)
    {
        Stdlib::ByteArray<IO::Epoll::Event> events;
        size_t eventsBufSize = 8 * 1024 * 1024;
        Stdlib::Error err;

        wg_.Add(1);

        Stdlib::ByteArray<u8> eventsBuf;
        if (!eventsBuf.ReserveAndUse(eventsBufSize)) {
            err = STDLIB_ERRNO_ERROR(ENOMEM);
            goto done;
        }

        while (!stopping_ && !(*stopping)) {
            err = epoll_.Wait(eventsBuf.GetBuf(), eventsBuf.GetSize(), events);
            if (err)
                goto done;

            for (size_t i = 0; i < events.GetSize(); i++) {
                if (stopping_ || *stopping)
                    goto done;

                auto& event = events[i];
                //Trace(0, "fd %d flags 0x%x\n", event.Fd, event.Flags);
                auto it = fd_co_map_.Lookup(event.fd_);
                if (it != fd_co_map_.End()) {

                    //f (event.Flags & (IO::Epoll::IN | IO::Epoll::OUT)) {
                        auto coset = *it;
                        for (auto it = coset->Begin(); it != coset->End();) {
                            auto co = *it;
                            it++;
                            if (co->GetSignalFd() == event.fd_)
                                co->ReadSignal();
                            co->Enter();
                        }

                        auto it2 = fd_co_map_.Lookup(event.fd_);
                        if (it2 != fd_co_map_.End())
                            epoll_.Mod(event.fd_, IO::Epoll::kIN | IO::Epoll::kOUT | IO::Epoll::kONESHOT);
                        else
                            epoll_.Del(event.fd_);
                    /*} else {
                        fd_co_map_.Remove(event.Fd);
                        epoll_.Del(event.Fd);
                    }*/
                } else
                    epoll_.Del(event.fd_);
            }
        }
        err = 0;
done:
        wg_.Done();
        return err;
    }

    Stdlib::Error CoEpoll::Create()
    {
        auto err = epoll_.Create();
        if (err)
            return err;
        wg_.Add(1);
        return 0;
    }

    void CoEpoll::Close()
    {
        stopping_ = true;
        if (epoll_.GetFd() >= 0) {
            wg_.Done();
            wg_.Wait();

            for (auto it = fd_co_map_.Begin(); it != fd_co_map_.End(); it++) {
                auto coset = *it;
                for (auto it2 = coset->Begin(); it2 != coset->End(); it2++) {
                    auto co = *it2;

                    Trace(CoEpollLL, "co %p ref %lu\n", co.Get(), co->GetRefCounter());
                }
                coset->Clear();
            }
            fd_co_map_.Clear();
            epoll_.Close();
        }
    }
}