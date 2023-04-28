#include <io/io_object.hpp>
#include <io/read_write_awaitable.hpp>

#include <unistd.h>

namespace NAsync {

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

} // namespace NAsync
