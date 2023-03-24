#pragma once

#include <polling/epoll.hpp>
#include <thread/pool.hpp>

namespace NAsync {

    struct TWithEpoll {
        TEpoll* Epoll = nullptr;
        TThreadPool* ThreadPool = nullptr;
    };

} // namespace NAsync