#include <io/io_object.hpp>

#include <sys/socket.h>
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

    TResult<int> Read(const TIoObject& ioObject, void* buf, int num, int flags) noexcept {
        int status;
        if (flags) {
            status = recv(ioObject.Fd(), buf, num, flags);
        } else {
            status = read(ioObject.Fd(), buf, num);
        }

        if (status < 0) {
            return std::error_code{errno, std::system_category()};
        }
        return status;
    }

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


} // namespace NAsync