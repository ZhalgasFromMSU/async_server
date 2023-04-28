#pragma once

#include <coro/coro_future.hpp>
#include <io/io_awaitable_base.hpp>
#include <util/result.hpp>

namespace NAsync {

    template<typename T>
    class TPromiseBase {
    public:
        std::suspend_always initial_suspend() noexcept {
            return std::suspend_always{};
        }

        std::suspend_never final_suspend() noexcept {
            return std::suspend_never{};
        }

        void unhandled_exception() {
            throw;
        }

        TCoroFuture<T> get_return_object() noexcept {
            return TCoroFuture<T>{*this};
        }

        template<std::derived_from<TWithEpoll> TIoAwaitable>
        TIoAwaitable&& await_transform(TIoAwaitable&& awaitable) noexcept {
            if (!awaitable.HasEpoll()) {
                awaitable.SetEpoll(Epoll);
            }
            if (!awaitable.HasThreadPool()) {
                awaitable.SetThreadPool(ThreadPool);
            }
            return std::forward<TIoAwaitable>(awaitable);
        }

        template<typename TSubResult>
        TCoroFuture<TSubResult>&& await_transform(TCoroFuture<TSubResult>&& subCoro) noexcept {
            if (!subCoro.Promise_.ThreadPool) {
                subCoro.Promise_.ThreadPool = ThreadPool;
            }
            if (!subCoro.Promise_.Epoll) {
                subCoro.Promise_.Epoll = Epoll;
            }
            return std::forward<TCoroFuture<TSubResult>>(subCoro);
        }

        // Runtime
        TEpoll* Epoll = nullptr;
        TThreadPool* ThreadPool = nullptr;
        std::coroutine_handle<> WaitingCoro;

        // Promise
        std::promise<T> StdPromise;
    };

    template<typename T>
    class TPromise: public TPromiseBase<T> {
    public:
        template<typename TResult>
            requires std::is_nothrow_constructible_v<T, TResult&&>
        void return_value(TResult&& res) noexcept {
            this->StdPromise.set_value(std::forward<TResult>(res));
            if (this->WaitingCoro) {
                if (this->ThreadPool) {
                    VERIFY(this->ThreadPool->EnqueJob(this->WaitingCoro));
                } else {
                    this->WaitingCoro();
                }
            }
        }

        template<typename TResult>
        void return_value(TResult&& res) {
            this->StdPromise.set_value(std::forward<TResult>(res));
            if (this->WaitingCoro) {
                if (this->ThreadPool) {
                    VERIFY(this->ThreadPool->EnqueJob(this->WaitingCoro));
                } else {
                    this->WaitingCoro();
                }
            }
        }
    };

    template<>
    class TPromise<void>: public TPromiseBase<void> {
    public:
        void return_void() noexcept {
            StdPromise.set_value();
            if (WaitingCoro) {
                if (this->ThreadPool) {
                    VERIFY(this->ThreadPool->EnqueJob(this->WaitingCoro));
                } else {
                    this->WaitingCoro();
                }
            }
        }
    };

    template<typename T>
    std::future<T> TCoroFuture<T>::Run() noexcept {
        auto future = Promise_.StdPromise.get_future();
        auto handle = std::coroutine_handle<TPromiseBase<T>>::from_promise(Promise_);
        if (Promise_.ThreadPool) {
            VERIFY(Promise_.ThreadPool->EnqueJob(std::move(handle)));
        } else {
            handle();
        }
        return future; // we first create future, because coroutine can be dead by the time we reach return statement
    }

    template<typename T>
    void TCoroFuture<T>::Run(std::future<T>& future) noexcept {
        future = Promise_.StdPromise.get_future();
        auto handle = std::coroutine_handle<TPromiseBase<T>>::from_promise(Promise_);
        if (Promise_.ThreadPool) {
            VERIFY(Promise_.ThreadPool->EnqueJob(std::move(handle)));
        } else {
            handle();
        }
    }

    template<typename T>
    void TCoroFuture<T>::SetEpoll(TEpoll* epoll) noexcept {
        Promise_.Epoll = epoll;
    }

    template<typename T>
    void TCoroFuture<T>::SetThreadPool(TThreadPool* threadPool) noexcept {
        Promise_.ThreadPool = threadPool;
    }

    template<typename T>
    void TCoroFuture<T>::BindWaitingCoro(std::coroutine_handle<> handle) noexcept {
        Promise_.WaitingCoro = std::move(handle);
    }

} // namespace NAsync
