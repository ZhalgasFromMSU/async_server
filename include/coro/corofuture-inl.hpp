#pragma once

#include "corofuture.hpp"
#include "promise.hpp"

namespace NAsync {

    template<typename T>
    TCoroFuture<T>::~TCoroFuture() {
        if (Promise_) {
            std::coroutine_handle<TPromise<T>>::from_promise(*Promise_).destroy();
        }
    }

    template<typename T>
    TCoroFuture<T>::TCoroFuture(TCoroFuture<T>&& other) noexcept {
        std::swap(Promise_, other.Promise_);
    }

    template<typename T>
    TCoroFuture<T>& TCoroFuture<T>::operator=(TCoroFuture<T>&& other) noexcept {
        std::swap(Promise_, other.Promise_);
        return *this;
    }

    template<typename T>
    TCoroFuture<T>& TCoroFuture<T>::Run() noexcept {
        std::coroutine_handle<TPromise<T>>::from_promise(*Promise_).resume();
        return *this;
    }

    template<typename T>
    bool TCoroFuture<T>::IsReady() noexcept {
        return Promise_->Ready.test();
    }

    template<typename T>
    TCoroFuture<T>& TCoroFuture<T>::Wait() noexcept {
        Promise_->Ready.wait(false);
        return *this;
    }

    template<typename T>
    T TCoroFuture<T>::Get() noexcept {
        Wait();
        if constexpr (std::is_same_v<T, void>) {
            VERIFY(Promise_->Ready.test());
            return;
        } else {
            return *std::move(Promise_->Value);
        }
    }

    template<typename T>
    bool TCoroFuture<T>::HasEpoll() const noexcept {
        return Promise_->Epoll != nullptr;
    }

    template<typename T>
    TCoroFuture<T>& TCoroFuture<T>::SetEpoll(TEpoll* epoll) noexcept {
        Promise_->Epoll = epoll;
        return *this;
    }

    template<typename T>
    bool TCoroFuture<T>::await_ready() const noexcept {
        return false;
    }

    template<typename T>
    void TCoroFuture<T>::await_suspend(std::coroutine_handle<> handle) noexcept {
        Promise_->Continuation = handle;
        Run();
    }

    template<typename T>
    T TCoroFuture<T>::await_resume() noexcept {
        return Get();
    }
} // namespace NAsync
