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
    class TPromiseBase {
    public:
        template<typename TReturnVal>
        void return_value(TReturnVal&& ret) {

        }
    };

    template<>
    class TPromiseBase<void> {
    public:
        void return_void() noexcept {

        }
    };

    template<typename T>
    class TPromise: public TPromiseBase<T> {
    public:

        template<typename... TArgs>
        TPromise(TEpoll* epoll, TArgs&&... /* args */) noexcept
            : Epoll_{epoll}
        {}

        template<typename... TArgs>
        TPromise(TEpoll* epoll, TThreadPool* threadPool, TArgs&&... /* args */) noexcept
            : Epoll_{epoll}
            , ThreadPool_{threadPool}
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
            return TPollableAwaitable<TPollable>{std::forward<TPollable>(pollable), Epoll_, ThreadPool_};
        }

        // TODO Add await transform for TCoroFuture for nested coroutines
        // template<typename TOther>
        // TFutureAwaitable<TOther> await_transform(TCoroFuture<TOther>&& future) noexcept {
        //     return ...;
        // }

    private:
        TEpoll* Epoll_;
        TThreadPool* ThreadPool_ = nullptr;
    };

} // namespace NAsync