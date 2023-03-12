#pragma once

#include <util/task.hpp>
#include <coro/promise.hpp>

namespace NAsync {

    template<typename T>
    class TCoroFuture {
    public:
        using promise_type = TPromise<T>;

        TCoroFuture(TPromise<T>* promise) noexcept
            : Promise_{promise}
        {}

        std::future<T> Run() {
            std::future<T> future = Promise_->get_future();
            auto handle = std::coroutine_handle<TPromise<T>>::from_promise(*Promise_);
            if (Promise_->ThreadPool != nullptr) {
                VERIFY(Promise_->ThreadPool->EnqueJob(handle));
            } else {
                handle();
            }
            return future;
        }

        template<CVoidToVoid TContinuationFunc>
        std::future<T> Run(TContinuationFunc then) {
            std::future<T> future = Promise_->get_future();
            auto handle = std::coroutine_handle<TPromise<T>>::from_promise(*Promise_);
            if (Promise_->ThreadPool != nullptr) {
                VERIFY(Promise_->ThreadPool->EnqueJob([handle = std::move(handle), then = std::move(then)] {
                    handle();
                    then();
                }));
            } else {
                handle();
                then();
            }
        }

        void SetEpoll(TEpoll* epoll) const noexcept {
            Promise_->Epoll = epoll;
        }

        void SetExecutor(TThreadPool* threadPool) const noexcept {
            Promise_->ThreadPool = threadPool;

    private:
        TPromise<T>* Promise_;
    };

}
