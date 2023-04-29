#pragma once

#include <polling/epoll.hpp>
#include <util/result.hpp>

#include <coroutine>
#include <future>

namespace NAsync {

    template<typename T>
    class TPromise;

    template<typename T>
    class TCoroFuture {
    public:
        using promise_type = TPromise<T>;

        void Run() noexcept;
        bool IsReady() noexcept;
        void Wait() noexcept;
        auto Get() noexcept;

        bool HasEpoll() const noexcept;
        void SetEpoll(TEpoll* epoll) noexcept;

        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> handle) noexcept;
        auto await_resume() noexcept;

    private:
        friend TPromise<T>;

        TCoroFuture(TPromise<T>& promise) noexcept
            : Promise_{promise}
        {}

        TPromise<T>& Promise_;
    };

}
