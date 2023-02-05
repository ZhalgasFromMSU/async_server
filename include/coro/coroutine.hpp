#pragma once

#include <coro/promise.hpp>

#include <future>

namespace NAsync {

    /*
        Rust-like Coroutines

        1) Coroutines live for as long as TCoroFuture object lives
        2) Coroutines live for as long as
    */
    template<typename T>
    class TCoroFuture: public std::future<T> {
    public:
        using promise_type = TPromise<T>;
        TCoroFuture(std::future<T> fut) noexcept {

        }

    private:
    };

}