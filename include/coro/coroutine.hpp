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

    private:
        TPromise<T>* Promise_;
    };

}