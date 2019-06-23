#pragma once

#include "epoll.h"

#include <common/base.h>
#include <common/io.h>

#include <common/sync/coroutine.h>
#include <common/sync/waitgroup.h>

#include <common/stdlib/base.h>
#include <common/stdlib/vector.h>
#include <common/stdlib/hash_map.h>
#include <common/stdlib/hash_set.h>
#include <common/stdlib/hash.h>
#include <common/stdlib/list_entry.h>
#include <common/stdlib/shared_ptr.h>
#include <common/stdlib/error.h>

namespace IO
{
    class CoEpoll
    {
    public:
        CoEpoll();
        virtual ~CoEpoll();

        Stdlib::Error Create();
        void Close();

        Stdlib::Error Add(int fd, Sync::CoroutinePtr co);
        Stdlib::Error Remove(int fd, Sync::CoroutinePtr co);

        Stdlib::Error EventLoop(bool *stopping);

    private:
        using CoSetType = Stdlib::HashSet<Sync::CoroutinePtr, 7, &Sync::HashCoroutinePtr>;
        Stdlib::HashMap<int, Stdlib::SharedPtr<CoSetType>, 997, &Stdlib::HashInt> fd_co_map_;

        CoEpoll(const CoEpoll& other) = delete;
        CoEpoll(CoEpoll&& other) = delete;
        CoEpoll& operator=(const CoEpoll& other) = delete;
        CoEpoll& operator=(CoEpoll&& other) = delete;

        Epoll epoll_;
        volatile bool stopping_;
        Sync::WaitGroup wg_;
    };
}