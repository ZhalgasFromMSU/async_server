#pragma once

#include <polling/epoll.hpp>
#include <thread/pool.hpp>
#include <util/result.hpp>

#include <coroutine>
#include <future>

namespace NAsync {

    template<typename T>
    class TPromiseBase;

    template<typename T>
    class TPromise;

    template<typename T>
    class TCoroFuture {
    public:
        using promise_type = TPromise<T>;

        std::future<T> Run() noexcept;

        void SetEpoll(TEpoll* epoll) noexcept;
        void SetThreadPool(TThreadPool* threadPool) noexcept;
        void BindWaitingCoro(std::coroutine_handle<> handle) noexcept;

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) noexcept {
            BindWaitingCoro(handle);
            Run(StdFuture_); // program will terminate if Run() was called before
        }

        T await_resume() noexcept {
            VERIFY(StdFuture_.wait_for(std::chrono::seconds::zero()) == std::future_status::ready);
            return StdFuture_.get();
        }

    private:
        friend TPromiseBase<T>;

        TCoroFuture(TPromiseBase<T>& promise) noexcept
            : Promise_{promise}
        {}

        // Overload without parameters can before std::future returns from function, leading to data races
        void Run(std::future<T>& future) noexcept;

        TPromiseBase<T>& Promise_;
        std::future<T> StdFuture_;
    };

}
