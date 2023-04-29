#pragma once

#include "corofuture.hpp"
#include "promise.hpp"

namespace NAsync {

    template<typename T>
    void TCoroFuture<T>::Run() noexcept {
        Promise_.Runtime->Execute(
            std::coroutine_handle<TPromise<T>>::from_promise(Promise_)
        );
    }

    template<typename T>
    bool TCoroFuture<T>::IsReady() noexcept {
        return Promise_.Ready.test();
    }

    template<typename T>
    void TCoroFuture<T>::Wait() noexcept {
        return Promise_.Ready.wait(false);
    }

    template<typename T>
    auto TCoroFuture<T>::Get() noexcept {
        struct TDeferred {
            ~TDeferred() {
                Promise.Runtime->Execute(
                    std::coroutine_handle<TPromise<T>>::from_promise(Promise)
                );
            }

            TPromise<T>& Promise;
        } defer {
            .Promise = Promise_
        };

        if constexpr (std::is_same_v<T, void>) {
            VERIFY(Promise_.Ready.test());
            return;
        } else {
            return *std::move(Promise_.Value);
        }
    }

    template<typename T>
    bool TCoroFuture<T>::HasRuntime() const noexcept {
        return Promise_.Runtime != nullptr;
    }

    template<typename T>
    void TCoroFuture<T>::SetRuntime(TRuntime* runtime) noexcept {
        Promise_.Runtime = runtime;
    }

    template<typename T>
    bool TCoroFuture<T>::await_ready() const noexcept {
        return false;
    }

    template<typename T>
    void TCoroFuture<T>::await_suspend(std::coroutine_handle<> handle) noexcept {
        Promise_.Continuation_ = handle;
    }

    template<typename T>
    auto TCoroFuture<T>::await_resume() noexcept {
        return Get();
    }
} // namespace NAsync
