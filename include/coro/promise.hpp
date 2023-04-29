#pragma once

#include <coro/corofuture.hpp>
#include <io/io_awaitable_base.hpp>
#include <type_traits>

namespace NAsync {

    template<typename T>
    struct is_coro_future : std::false_type {};

    template<typename T>
    struct is_coro_future<TCoroFuture<T>> : std::true_type {};

    template<typename T>
    class TPromiseBase {
    public:
        void unhandled_exception() noexcept;
        std::suspend_always initial_suspend() noexcept;
        std::suspend_always final_suspend() noexcept; // postpone promise destruction, until someone retrieves value
                                                      // no allocation for future state though

        template<std::derived_from<TWithEpoll> TIoAwaitable>
        TIoAwaitable&& await_transform(TIoAwaitable&& awaitable) noexcept;

        template<typename TSubFuture>
            requires (is_coro_future<TSubFuture>::value)
        TSubFuture&& await_transform(TSubFuture&& subCoro) noexcept;

        // Members variables
        TRuntime* Runtime = nullptr;
        std::atomic_flag Ready;
        std::coroutine_handle<> Continuation_;
    };

    template<typename T>
    class TPromise: public TPromiseBase<T> {
    public:
        template<typename TResult>
        void return_value(TResult&& res) noexcept;

        TCoroFuture<T> get_return_object() noexcept;

        // Member variables
        std::optional<T> Value;
    };

    template<>
    class TPromise<void>: public TPromiseBase<void> {
    public:
        void return_void() noexcept;

        TCoroFuture<void> get_return_object() noexcept;
    };

} // namespace NAsync
