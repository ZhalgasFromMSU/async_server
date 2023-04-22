#include "converter.hpp"
#include <net/accept_connect_awiatable.hpp>

namespace NAsync {

    // TConnectAwaitable
    bool TConnectAwaitable::await_ready() noexcept {
        NPrivate::TConverter converter{*Socket_.RemoteAddress()};
        int status = connect(Socket_.Fd(), converter.Raw(), converter.Size());
        if (status == -1) {
            if (errno & (EINPROGRESS | EAGAIN)) {
                return false;
            }
            Error_ = std::error_code{errno, std::system_category()};
        } else {
            Error_ = std::error_code{};
        }
        return true;
    }

    void TConnectAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        if (ThreadPool) {
            Epoll->Watch(TEpoll::EMode::kWrite, Socket_, [this, handle] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->Watch(TEpoll::EMode::kWrite, Socket_, handle);
        }
    }

    std::error_code TConnectAwaitable::await_resume() noexcept {
        if (Error_) {
            return std::move(*Error_);
        }
        int error;
        socklen_t size = sizeof(error);
        int status = getsockopt(Socket_.Fd(), SOL_SOCKET, SO_ERROR, &error, &size);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }

        if (error != 0) {
            return std::error_code{error, std::system_category()};
        }

        return std::error_code{};
    }

} // namespace NAsync
