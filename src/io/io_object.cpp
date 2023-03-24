#include <io/io_object.hpp>
#include <util/result.hpp>

#include <unistd.h>

namespace NAsync {

    TIoObject::TIoObject(int fd) noexcept
        : Fd_(fd)
    {
        VERIFY(Fd_ >= 0);
    }

    TIoObject& TIoObject::operator=(TIoObject&& other) noexcept {
        std::swap(Fd_, other.Fd_);
        return *this;
    }

    TIoObject::TIoObject(TIoObject&& other) noexcept {
        std::swap(Fd_, other.Fd_);
    }

    TIoObject::~TIoObject() noexcept {
        if (Fd_ >= 0) {
            VERIFY_SYSCALL(close(Fd_) >= 0);
        }
    }

    int TIoObject::Fd() const noexcept {
        return Fd_;
    }

} // namespace NAsync