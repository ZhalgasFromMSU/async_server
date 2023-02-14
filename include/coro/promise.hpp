#pragma once

#include <io/pollable_object.hpp>
#include <polling/epoll.hpp>
#include <thread/pool.hpp>
#include <coro/awaitable.hpp>

#include <coroutine>

namespace NAsync {

    template<typename T>
    class TCoroFuture;

    template<typename T>
    class TPromiseBase: public std::promise<T> {
    public:
        template<typename TResult>
        void return_value(TResult&& result) {
            std::promise<T>::set_value(std::forward<TResult>(result));
        }
    };

    template<>
    class TPromiseBase<void>: public std::promise<void> {
    public:
        void return_void() noexcept {
            set_value();
        }
    };

    template<typename T>
    class TPromise: public TPromiseBase<T> {
    public:
        template<typename... TArgs>
        TPromise(TEpoll* epoll, TArgs&&... /* args */) noexcept
            : Epoll{epoll}
        {}

        template<typename... TArgs>
        TPromise(TEpoll* epoll, TThreadPool* threadPool, TArgs&&... /* args */) noexcept
            : Epoll{epoll}
            , ThreadPool{threadPool}
        {}

        // these two constructors are for member function coroutines
        template<typename TThis, typename... TArgs>
        TPromise(TThis&& obj, TEpoll* epoll, TArgs&&...) noexcept
            : Epoll{epoll}
        {}

        template<typename TThis, typename... TArgs>
        TPromise(TThis&& obj, TEpoll* epoll, TThreadPool* threadPool, TArgs&&...) noexcept
            : Epoll{epoll}
            , ThreadPool{threadPool}
        {}

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {
            throw;
        }

        TCoroFuture<T> get_return_object() noexcept {
            return TCoroFuture<T>{this};
        }

        template<CPollable TPollable>
        TPollableAwaitable<TPollable> await_transform(TPollable&& pollable) noexcept {
            return TPollableAwaitable<TPollable>{std::forward<TPollable>(pollable), Epoll, ThreadPool};
        }

        // TODO Add await transform for TCoroFuture for nested coroutines
        // template<typename TOther>
        // TFutureAwaitable<TOther> await_transform(TCoroFuture<TOther>&& future) noexcept {
        //     return ...;
        // }

        TEpoll* Epoll;
        TThreadPool* ThreadPool = nullptr;
    };

} // namespace NAsync