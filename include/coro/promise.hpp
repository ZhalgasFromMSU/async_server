#pragma once

#include <io/pollable_object.hpp>
#include <polling/epoll.hpp>
#include <thread/pool.hpp>
#include <coro/awaitable.hpp>

#include <coroutine>
#include <iostream>

namespace NAsync {

    template<typename T>
    class TCoroFuture;

    template<typename T>
    class TPromiseBase: protected std::promise<T> {
    public:

        template<typename... TArgs>
        TPromiseBase(TEpoll& epoll, TThreadPool& threadPool, TArgs&&... args) noexcept
            : Epoll_{epoll}
            , ThreadPool_{threadPool}
        {}

        std::suspend_never initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {
            throw;
        }

        TCoroFuture<T> get_return_object() noexcept {
            return TCoroFuture<T>{get_future()};
        }

        template<CPollable TPollable>
        TPollableAwaitable<TPollable> await_transform(TPollable&& pollable) noexcept {
            return TPollableAwaitable<TPollable>{std::forward<TPollable>(pollable), Epoll_, ThreadPool_};
        }

        // TODO Add await transform for TCoroFuture for nested coroutines

    private:
        using std::promise<T>::get_future;

        TEpoll& Epoll_;
        TThreadPool& ThreadPool_;
    };

    template<typename T>
    class TPromise: public TPromiseBase<T> {
    public:
        using TPromiseBase<T>::TPromiseBase;

        template<typename TReturnValue>
        void return_value(TReturnValue&& ret) {
            set_value(std::forward(ret));
        }
    };

    template<>
    class TPromise<void>: public TPromiseBase<void> {
    public:
        using TPromiseBase<void>::TPromiseBase;

        void return_void() {
            set_value();
        }
    };

} // namespace NAsync