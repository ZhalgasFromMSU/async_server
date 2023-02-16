#pragma once

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

        bool SetExecutor(TThreadPool* threadPool) const {
            if (Promise_->ThreadPool) {
                return false;
            }
            Promise_->ThreadPool = threadPool;
            return true;
        }

        bool SetEpoll(TEpoll* epoll) const {
            if (Promise_->Epoll) {
                return false;
            }
            Promise_->Epoll = epoll;
            return true;
        }

    private:
        TPromise<T>* Promise_;
    };

}
