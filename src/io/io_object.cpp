#include <io/io_object.hpp>

#include <sys/socket.h>
#include <unistd.h>

namespace NAsync {

    TIoObject::TIoObject(int fd) noexcept
        : Fd_(fd)
    {}

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

    TResult<int> TIoObject::Write(const void* buf, int numBytesToWrite, int flags) const noexcept {
        int status;
        if (flags) {
            status = send(Fd_, buf, numBytesToWrite, flags);
        } else {
            status = write(Fd_, buf, numBytesToWrite);
        }
        if (status < 0) {
            return std::error_code{errno, std::system_category()};
        }
        return status;
    }

    TResult<int> TIoObject::Read(void* buf, int numBytesToRead, int flags) const noexcept {
        int status;
        if (flags) {
            status = recv(Fd_, buf, numBytesToRead, flags);
        } else {
            status = read(Fd_, buf, numBytesToRead);
        }
        if (status < 0) {
            return std::error_code{errno, std::system_category()};
        }
        return status;
    }

} // namespace NAsync