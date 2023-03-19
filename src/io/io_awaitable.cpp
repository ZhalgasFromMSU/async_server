#include <io/io_awaitable.hpp>

namespace NAsync {

    // TReadAwaitable
    bool TReadAwaitable::await_ready() noexcept {
        TResult<int> result = Read(IoObject_, Buf_, Num_, Flags_);
        if (!result && (result.Error().value() == EWOULDBLOCK || result.Error().value() == EAGAIN)) {
            return false;
        }
        Result_ = std::move(result);
        return true;
    }

    void TReadAwaitable::await_suspend(std::coroutine_handle<> handle) const noexcept {
        if (ThreadPool) {
            Epoll->WatchForRead(IoObject_, [&] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->WatchForRead(IoObject_, handle);
        }
    }

    TResult<int> TReadAwaitable::await_resume() noexcept {
        if (Result_.has_value()) {
            return std::move(*Result_);
        }
        return Read(IoObject_, Buf_, Num_, Flags_);
    }

    // TWriteAwaitable
    bool TWriteAwaitable::await_ready() noexcept {
        TResult<int> result = Write(IoObject_, Buf_, Num_, Flags_);
        if (!result && (result.Error().value() == EWOULDBLOCK || result.Error().value() == EAGAIN)) {
            return false;
        }
        Result_ = std::move(result);
        return true;
    }

    void TWriteAwaitable::await_suspend(std::coroutine_handle<> handle) const noexcept {
        if (ThreadPool) {
            Epoll->WatchForWrite(IoObject_, [&] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->WatchForWrite(IoObject_, handle);
        }
    }

    TResult<int> TWriteAwaitable::await_resume() noexcept {
        if (Result_.has_value()) {
            return std::move(*Result_);
        }
        return Write(IoObject_, Buf_, Num_, Flags_);
    }

}