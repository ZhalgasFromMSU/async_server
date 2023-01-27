#pragma once

#include <polling/epoll.hpp>
#include <thread/pool.hpp>

#include <coroutine>
#include <future>

namespace NAsync {

    /*
        Rust-like Coroutines

        1) Coroutines live for as long as TCoroFuture object lives
        2) Coroutines live for as long as
    */
    template<typename T>
    class TCoroFuture: protected std::future<T> {
    public:
        using std::future<T>::future;
    };

    template<typename T>
    class TPromiseBase: protected std::promise<T> {
    public:
        TPromiseBase(TEpoll& epoll, TThreadPool& threadPool) noexcept
            : Epoll_{epoll}
            , ThreadPool_{threadPool}
        {}

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {
            throw;
        }

        TCoroFuture<T> get_return_object() noexcept {
            return TCoroFuture<T>{*this};
        }

    private:
        TEpoll& Epoll_;
        TThreadPool& ThreadPool_;
    };

    template<typename T>
    class TPromise: public TPromiseBase<T> {
    public:
        template<typename TReturnValue>
        void return_value(TReturnValue&& ret) {
            set_value(std::forward(ret));
        }
    };

    template<>
    class TPromise<void>: public TPromiseBase<void> {
    public:
        void return_void() {
            set_value();
        }
    };

}