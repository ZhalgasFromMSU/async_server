#pragma once

#include <io/pollable_object.hpp>
#include <coro/forward.hpp>

namespace NAsync {

    template<typename T>
    concept COptional = std::same_as<T, std::optional<typename T::value_type>>;

    template<typename T>
    concept CPollable = requires(
        const T t,
        TEpoll& epoll,
        TEpoll::TCallback callback
    ) {
        { t.Try() } noexcept -> COptional;
        { t.ScheduleToEpoll(epoll, callback) } -> std::same_as<std::error_code>; // TODO make this method noexcept also
    };

    template<CPollable T>
    class TPollableAwaitable {
    public:
        using TOptionalResult = std::invoke_result_t<decltype(&T::Try), T*>; // std::optional<T>

        TPollableAwaitable(const T& pollableObject, TEpoll& epoll, TThreadPool& threadPool) noexcept
            : PollableObject_{pollableObject}
            , Epoll_{epoll}
            , ThreadPool_{threadPool}
        {}

        bool await_ready() noexcept {
            MaybeResult_ = std::move(PollableObject_.Try());
            return MaybeResult_.has_value();
        }

        void await_suspend(std::coroutine_handle<> handle) noexcept {
            PollableObject_.ScheduleToEpoll(Epoll_, [this, handle = std::move(handle)] {
                VERIFY(ThreadPool_.EnqueJob(std::move(handle)));
            });
        }

        TOptionalResult::value_type await_resume() noexcept {
            if (MaybeResult_.has_value()) {
                return std::move(*MaybeResult_);
            }
            return *PollableObject_.Try();
        }

    private:
        const T& PollableObject_;
        TEpoll& Epoll_;
        TThreadPool& ThreadPool_;
        TOptionalResult MaybeResult_;
    };

    template<typename T>
    class TFutureAwaitable {
    public:
        TFutureAwaitable(TCoroFuture<T>& coroFuture) noexcept
            : CoroFuture_{coroFuture}
        {}

        bool await_ready() noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<> handle) noexcept {
            CoroFuture_.BindWaitingCoro(std::move(handle));
            CoroResult_ = CoroFuture_.Run();
            return !IsReady(CoroFuture_); // no need to suspend if ready
        }

        T await_resume() noexcept {
            VERIFY(IsReady(CoroFuture_));
            return CoroResult_.get();
        }

    private:
        template<typename TOther>
        static bool IsReady(std::future<TOther>& future) noexcept {
            return future.wait_for(std::chrono::seconds::zero()) == std::future_status::ready;
        }

        TCoroFuture<T>& CoroFuture_;
        std::future<T> CoroResult_;
    };

} // namespace NAsync