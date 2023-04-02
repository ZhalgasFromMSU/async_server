#include <io/write.hpp>

#include <sys/socket.h>

namespace NAsync {

    TResult<int> Write(const TIoObject& ioObject, const void* buf, int num, int flags) noexcept {
        int status;
        if (flags) {
            status = send(ioObject.Fd(), buf, num, flags);
        } else {
            status = write(ioObject.Fd(), buf, num);
        }

        if (status < 0) {
            return std::error_code{errno, std::system_category()};
        }
        return status;
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
            Epoll->WatchForWrite(IoObject_, [this, handle] {
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

} // namespace NAsync