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

    bool TConnectAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        return Suspend(TEpoll::EMode::kWrite, Socket_, handle);
    }

    std::error_code TConnectAwaitable::await_resume() noexcept {
        if (Error_) {
            return std::move(*Error_);
        }
        int acceptResult;
        socklen_t size = sizeof(acceptResult);
        int status = getsockopt(Socket_.Fd(), SOL_SOCKET, SO_ERROR, &acceptResult, &size);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }

        if (acceptResult != 0) {
            return std::error_code{acceptResult, std::system_category()};
        }

        return std::error_code{};
    }

} // namespace NAsync
