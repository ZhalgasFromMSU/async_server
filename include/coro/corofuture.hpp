#pragma once

#include <polling/epoll.hpp>

namespace NAsync {

    template<typename T>
    class TPromise;

    template<typename T>
    class TCoroFuture {
    public:
        using promise_type = TPromise<T>;

        // Constructors
        TCoroFuture() noexcept = default;
        TCoroFuture(TCoroFuture&&) noexcept;
        TCoroFuture& operator=(TCoroFuture&&) noexcept;
        TCoroFuture(const TCoroFuture&) = delete;
        TCoroFuture& operator=(const TCoroFuture&) = delete;
        ~TCoroFuture();

        // Runtime control
        TCoroFuture& Run() noexcept;
        TCoroFuture& Wait() noexcept;
        bool HasEpoll() const noexcept;
        TCoroFuture& SetEpoll(TEpoll* epoll) noexcept;

        // Accessors
        bool IsReady() const noexcept;
        T Get() noexcept;
        const T* Peek() const noexcept // return nullptr if not ready
            requires (!std::is_same_v<T, void>);

        // Nested coro
        bool await_ready() const noexcept;
        void await_suspend(std::coroutine_handle<> handle) noexcept;
        T await_resume() noexcept;

    private:
        friend TPromise<T>;

        TCoroFuture(TPromise<T>* promise) noexcept
            : Promise_{promise}
        {}

        TPromise<T>* Promise_ = nullptr;
    };

}
