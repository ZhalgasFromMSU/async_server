#pragma once

#include <coro/promise.hpp>

#include <future>

namespace NAsync {

    template<typename T>
    class TCoroFuture {
    public:
        using promise_type = TPromise<T>;

        TCoroFuture(promise_type* promise) noexcept
            : Promise_{promise}
        {}

        std::future<T> Run() noexcept {
            auto handle = std::coroutine_handle<promise_type>::from_promise(Promise_);
            if (Promise_.Executor) {
                VERIFY(Promise_->ThreadPool->EnqueJob(std::move(handle)));
            } else {
                handle();
            }
            return {};
        }

        void SetExecutor(TThreadPool* threadPool) const noexcept {
            Promise_->ThreadPool = threadPool;
        }

    private:
        promise_type* Promise_;
    };

}