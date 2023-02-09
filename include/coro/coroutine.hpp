#pragma once

#include <coro/promise.hpp>

#include <future>

namespace NAsync {

    template<typename T>
    class TCoroFuture {
    public:
        using promise_type = TPromise<T>;

        TCoroFuture(TPromise<T>* promise) noexcept
            : Promise_{promise}
        {}

        void Run() noexcept {

        }

        void SetExecutor() noexcept {

        }

    private:
        TPromise<T>* Promise_;
    };

}