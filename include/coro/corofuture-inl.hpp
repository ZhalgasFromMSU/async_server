#pragma once

#include "corofuture.hpp"
#include "promise.hpp"
#include <iostream>

namespace NAsync {

    template<typename T>
    void TCoroFuture<T>::Run() noexcept {
        std::coroutine_handle<TPromise<T>>::from_promise(Promise_).resume();
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
                std::coroutine_handle<TPromise<T>>::from_promise(Promise).destroy();
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
    bool TCoroFuture<T>::HasEpoll() const noexcept {
        return Promise_.Epoll != nullptr;
    }

    template<typename T>
    void TCoroFuture<T>::SetEpoll(TEpoll* epoll) noexcept {
        Promise_.Epoll = epoll;
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
