#pragma once

#include <thread/pool.hpp>
#include <polling/epoll.hpp>

namespace NAsync {

    class TRuntime {
    public:
        TRuntime(size_t numThreads = std::thread::hardware_concurrency()) noexcept;
        ~TRuntime();

        void Start() noexcept;
        void Stop() noexcept;

        void Execute(std::coroutine_handle<> handle) noexcept;
        bool Schedule(TEpoll::EMode mode, const TIoObject& io, std::coroutine_handle<> handle) noexcept;

    private:
        std::optional<TThreadPool> Tp_;
        TEpoll Epoll_; // polls and schedules tasks on thread pool
    };

} // namespace NAsync
