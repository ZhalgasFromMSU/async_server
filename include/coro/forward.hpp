#pragma once

#include <polling/epoll.hpp>
#include <thread/pool.hpp>

#include <coroutine>

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

    private:
        friend TPromiseBase<T>;

        TCoroFuture(TPromiseBase<T>& promise) noexcept
            : Promise_{promise}
        {}

        TPromiseBase<T>& Promise_;
    };

}