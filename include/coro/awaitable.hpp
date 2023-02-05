#pragma once

#include <io/pollable_object.hpp>

#include <coroutine>

namespace NAsync {

    template<CPollable T>
    class TPollableAwaitable {
    public:
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
                VERIFY(ThreadPool_.EnqueJob(handle));
            });
        }

        TResult<int> await_resume() noexcept {
            if (MaybeResult_.has_value()) {
                return std::move(*MaybeResult_);
            }
            return *PollableObject_.Try();
        }

    private:
        const T& PollableObject_;
        TEpoll& Epoll_;
        TThreadPool& ThreadPool_;
        std::optional<TResult<int>> MaybeResult_;
    };

} // namespace NAsync