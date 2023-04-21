#pragma once

#include <io/io_object.hpp>
#include <io/well_known_structs.hpp>
#include <util/task.hpp>

#include <unordered_map>
#include <memory>
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

        // static TResult<std::shared_ptr<TEpoll>> Create() noexcept;
        TEpoll() noexcept;
        ~TEpoll();

        void Start() noexcept;
        void Stop() noexcept;

        // All callbacks are executed synchronously
        std::error_code Watch(const TIoObject& io, std::unique_ptr<ITask> callback, EMode mode) noexcept;

        template<CVoidToVoid TFunc>
        std::error_code Watch(const TIoObject& io, TFunc&& callback, EMode mode) noexcept {
            return Watch(io, new TTask(std::forward<TFunc>(callback)), mode);
        }

    private:
        TEventFd EventFd_;
        std::unordered_map<int, std::unique_ptr<ITask>> Callbacks_; // <fd, callback>

        std::mutex Mutex_;
        std::atomic<bool> Stopped_ = false;
        std::thread Worker_;
    };

} // namespace NAsync
