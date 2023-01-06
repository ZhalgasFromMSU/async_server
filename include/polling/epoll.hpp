#pragma once

#include <thread_pool/pool.hpp>
#include <io/io_object.hpp>
#include <util/list.hpp>

#include <system_error>
#include <functional>
#include <list>
#include <future>

namespace NAsync {

    struct TEpollError: std::system_error {
        using std::system_error::system_error;
    };

    // Epoll can work asynchronously. E.g. new fds can be added in already running epoll_wait
    // Thus epoll_wait runs infinitely in background
    class TEpoll {
    public:
        using TCallback = std::function<void()>;

        TEpoll() noexcept;
        ~TEpoll() noexcept;

        // All callbacks are executed synchronously
        std::error_code WatchForRead(int fd, TCallback callback) noexcept;
        std::error_code WatchForWrite(int fd, TCallback callback) noexcept;

    private:
        void PollFunc() noexcept;
        void EventFdCallback() noexcept;

        constexpr static size_t EpollBuffSize_ = 1024;
        int EpollFd_;
        TIoObject EventFd_;

        TList<std::pair<int, TCallback>> Callbacks_;

        std::recursive_mutex EpollMutex_;
        std::atomic<bool> ShouldFinish_ = false;

        std::future<void> EpollFuture_;
    };

} // namespace NAsync