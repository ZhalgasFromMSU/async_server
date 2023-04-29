#include <io/io_object.hpp>
#include <polling/epoll.hpp>

#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

namespace NAsync {

    namespace {

        template<bool Read>
        TResult<int> Do(const TIoObject& ioObject,
                        std::conditional_t<Read, void*, const void*> buf, 
                        int num,
                        int flags) {
            int status;
            if constexpr (Read) {
                if (flags) {
                    status = recv(ioObject.Fd(), buf, num, flags);
                } else {
                    status = read(ioObject.Fd(), buf, num);
                }
            } else {
                if (flags) {
                    status = send(ioObject.Fd(), buf, num, flags);
                } else {
                    status = write(ioObject.Fd(), buf, num);
                }
            }
            if (status < 0) {
                return std::error_code{errno, std::system_category()};
            }
            return status;
        }

    } // namespace

    // TIoObject
    TIoObject::TIoObject(int fd) noexcept
        : Fd_(fd)
    {
        VERIFY(Fd_ >= 0 || Fd_ == -1);
    }

    TIoObject& TIoObject::operator=(TIoObject&& other) noexcept {
        if (Fd_ >= 0) {
            VERIFY_SYSCALL(close(Fd_) >= 0);
        }
        Fd_ = other.Fd_;
        other.Fd_ = -1;
        return *this;
    }

    TIoObject::TIoObject(TIoObject&& other) noexcept
        : Fd_{other.Fd_}
    {
        other.Fd_ = -1;
    }

    TIoObject::~TIoObject() noexcept {
        if (Fd_ >= 0) {
            VERIFY_SYSCALL(close(Fd_) >= 0);
        }
    }

    int TIoObject::Fd() const noexcept {
        return Fd_;
    }

    TReadAwaitable TIoObject::Read(void* buf, int num, int flags) const noexcept {
        return TReadAwaitable{*this, buf, num, flags};
    }

    TWriteAwaitable TIoObject::Write(const void* buf, int num, int flags) const noexcept {
        return TWriteAwaitable{*this, buf, num, flags};
    }
    
    // TReadAwaitable
    bool TReadAwaitable::await_ready() noexcept {
        TResult<int> result = Do</* Read */true>(IoObject_, Buf_, Num_, Flags_);
        if (!result && (result.Error().value() & (EWOULDBLOCK | EAGAIN))) {
            return false;
        }
        Result_ = std::move(result);
        return true;
    }

    bool TReadAwaitable::await_suspend(std::coroutine_handle<> handle) const noexcept {
        return !Epoll_->Watch(TEpoll::EMode::kRead, IoObject_, std::move(handle));
    }

    TResult<int> TReadAwaitable::await_resume() noexcept {
        if (Result_.has_value()) {
            return std::move(*Result_);
        }
        return Do</* Read */true>(IoObject_, Buf_, Num_, Flags_);
    }

    // TWriteAwaitable
    bool TWriteAwaitable::await_ready() noexcept {
        TResult<int> result = Do</*Write*/false>(IoObject_, Buf_, Num_, Flags_);
        if (!result && (result.Error().value() & (EWOULDBLOCK | EAGAIN))) {
            return false;
        }
        Result_ = std::move(result);
        return true;
    }

    bool TWriteAwaitable::await_suspend(std::coroutine_handle<> handle) const noexcept {
        return !Epoll_->Watch(TEpoll::EMode::kWrite, IoObject_, std::move(handle));
    }

    TResult<int> TWriteAwaitable::await_resume() noexcept {
        if (Result_.has_value()) {
            return std::move(*Result_);
        }
        return Do</*Write*/false>(IoObject_, Buf_, Num_, Flags_);
    }

} // namespace NAsync
