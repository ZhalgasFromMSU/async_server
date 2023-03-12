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

        TEpoll* Epoll = nullptr;
        TThreadPool* ThreadPool = nullptr;
    };

} // namespace NAsync