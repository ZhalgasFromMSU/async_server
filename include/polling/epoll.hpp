#pragma once

#include <io/well_known_structs.hpp>
#include <util/task.hpp>
#include <thread/wait_group.hpp>

#include <unordered_map>
#include <thread>

namespace NAsync {

    // Epoll can work asynchronously. E.g. new fds can be added in already running epoll_wait
    // Thus epoll_wait runs infinitely in background
    class TEpoll : public TIoObject {
    public:
        enum class EMode {
            kRead,
            kWrite,
        };

        TEpoll() noexcept;
        ~TEpoll();

        void Start() noexcept;
        void Finish() noexcept;

        // All callbacks are executed synchronously
        std::error_code Watch(EMode mode, const TIoObject& io, TJob callback) noexcept;

    private:
        TEventFd EventFd_; // to stop epoll_wait
        std::atomic_flag Stopped_;

        std::mutex Mut_;
        std::unordered_map<int, TJob> Cbs_; // <fd, callback>

        std::jthread Worker_;
    };

} // namespace NAsync
