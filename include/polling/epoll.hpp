#pragma once

#include <io/well_known_structs.hpp>
#include <thread/pool.hpp>

#include <mutex>

namespace NAsync {

    // Epoll can work asynchronously. E.g. new fds can be added in already running epoll_wait
    // Thus epoll_wait runs infinitely in background
    class TEpoll : public TIoObject {
    public:
        enum class EMode {
            kRead,
            kWrite,
        };

        TEpoll(size_t additionalPoolSize = 0) noexcept; // if set, callbacks are executed on another thread pool
        ~TEpoll();

        void Start() noexcept;
        void Finish() noexcept;

        // All callbacks are executed synchronously
        std::error_code Watch(EMode mode, const TIoObject& io, TJob callback) noexcept;

    private:
        TEventFd EventFd_; // to stop epoll_wait
        std::atomic_flag Stopped_;

        std::recursive_mutex Mut_;
        std::unordered_map<int, TJob> Cbs_; // <fd, callback>

        std::jthread Worker_;
        std::optional<TThreadPool> CbsPool_; // callbacks are executed here
    };

} // namespace NAsync
