#pragma once

#include <polling/epoll.hpp>
#include <thread/pool.hpp>

namespace NAsync {

    class TWithEpoll {
    public:
        inline bool HasEpoll() const noexcept {
            return Epoll_ != nullptr;
        }

        inline bool HasThreadPool() const noexcept {
            return ThreadPool_ != nullptr;
        }

        inline void SetEpoll(TEpoll* epoll) noexcept {
            Epoll_ = epoll;
        }

        inline void SetThreadPool(TThreadPool* tp) noexcept {
            ThreadPool_ = tp;
        }

        bool Suspend(TEpoll::EMode mode, const TIoObject& io, std::coroutine_handle<> handle) const;

    private:
        TEpoll* Epoll_ = nullptr;
        TThreadPool* ThreadPool_ = nullptr;
    };

} // namespace NAsync
