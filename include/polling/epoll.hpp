#pragma once

#include <io/io_object.hpp>
#include <io/well_known_structs.hpp>

#include <unordered_map>
#include <system_error>
#include <functional>
#include <list>
#include <future>

namespace NAsync {

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

        // Should probably add RemoveFromWatchlist method, to skip callback execution if socket was closed

    private:
        void PollLoop() noexcept;

        TIoObject EpollFd_;
        TEventFd EventFd_;
        std::atomic<bool> EpollStopped_;
        std::thread EpollBackgroundThread_;

        std::mutex CallbacksMutex_;
        std::list<TCallback> Callbacks_;
        std::unordered_map<int, std::list<TCallback>::iterator> FdIteratorMapping_;
    };

} // namespace NAsync