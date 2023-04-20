#pragma once

#include <io/io_object.hpp>
#include <io/well_known_structs.hpp>
#include <util/task.hpp>
#include <util/result.hpp>

#include <unordered_map>
#include <system_error>
#include <functional>
#include <list>
#include <future>

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
        ~TEpoll() noexcept;

        // All callbacks are executed synchronously
        std::error_code Watch(const TIoObject& io, std::unique_ptr<ITask> task, EMode mode) noexcept;

        template<CVoidToVoid TFunc>
        std::error_code Watch(const TIoObject& io, TFunc&& func, EMode mode) noexcept {
            return Watch(io, new TTask(std::forward<TFunc>(func)), mode);
        }

    private:
        void PollLoop() noexcept;

        TEventFd EventFd_;
        std::atomic<bool> EpollStopped_;
        std::thread EpollBackgroundThread_;

        std::mutex CallbacksMutex_;
        std::list<std::unique_ptr<ITask>> Callbacks_;
        std::unordered_map<int, std::list<TCallback>::iterator> FdIteratorMapping_;
    };

} // namespace NAsync
