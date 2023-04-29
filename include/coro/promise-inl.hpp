#pragma once

#include "corofuture.hpp"
#include "promise.hpp"
#include <coroutine>

namespace NAsync {

    // TPromiseBase<T>
    template<typename T>
    std::suspend_always TPromiseBase<T>::initial_suspend() noexcept {
        return {};
    }

    template<typename T>
    std::suspend_always TPromiseBase<T>::final_suspend() noexcept {
        if (Continuation_) {
            Continuation_.resume();
        }
        return {};
    }

    template<typename T>
    void TPromiseBase<T>::unhandled_exception() noexcept {
        std::terminate();
    }

    template<typename T>
    template<std::derived_from<TWithEpoll> TIoAwaitable>
    TIoAwaitable&& TPromiseBase<T>::await_transform(TIoAwaitable&& awaitable) noexcept {
        if (!awaitable.HasEpoll()) {
            awaitable.SetEpoll(Epoll);
        }
        return std::forward<TIoAwaitable>(awaitable);
    }

    template<typename T>
    template<typename TSubFuture>
        requires (is_coro_future<TSubFuture>::value)
    TSubFuture&& TPromiseBase<T>::await_transform(TSubFuture&& subCoro) noexcept {
        if (!subCoro.HasEpoll()) {
            subCoro.SetEpoll(Epoll);
        }
        return std::forward<TSubFuture>(subCoro);
    }

    // TPromise<T>
    template<typename T>
    template<typename TResult>
    void TPromise<T>::return_value(TResult&& result) noexcept {
        Value.emplace(std::forward<TResult>(result));
        TPromiseBase<T>::Ready.test_and_set();
        TPromiseBase<T>::Ready.notify_all();
    }

    template<typename T>
    TCoroFuture<T> TPromise<T>::get_return_object() noexcept {
        return TCoroFuture<T>{*this};
    }

    // TPromise<void>
    inline void TPromise<void>::return_void() noexcept {
        TPromiseBase<void>::Ready.test_and_set();
        TPromiseBase<void>::Ready.notify_all();
    }

    inline TCoroFuture<void> TPromise<void>::get_return_object() noexcept {
        return TCoroFuture<void>{*this};
    }

} // namespace NAsync
