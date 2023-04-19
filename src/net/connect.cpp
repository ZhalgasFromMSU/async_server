#include "helpers.hpp"
#include <net/accept_connect_awiatable.hpp>

namespace NAsync {

    namespace {
        int Connect(int sockFd, const TSocketAddress& dest) {
            int status;
            if (std::holds_alternative<TIPv4SocketAddress>(dest)) {
                sockaddr_in raw;
                NPrivate::ConvertSockaddr(dest, &raw);
                status = connect(sockFd, NPrivate::ConstPtrCast<sockaddr>(&raw), sizeof(raw));
            } else if (std::holds_alternative<TIPv6SocketAddress>(dest)) {
                sockaddr_in6 raw;
                NPrivate::ConvertSockaddr(dest, &raw);
                status = connect(sockFd, NPrivate::ConstPtrCast<sockaddr>(&raw), sizeof(raw));
            } else {
                VERIFY(false);
            }

            return status;
        }
    }

    // TConnectAwaitable
    bool TConnectAwaitable::await_ready() noexcept {
        int status = Connect(Socket_.Fd(), Dest_);
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
            Epoll->WatchForWrite(Socket_, [this, handle] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->WatchForWrite(Socket_, handle);
        }
    }
    std::error_code TConnectAwaitable::await_resume() noexcept {
        if (Error_) {
            return std::move(*Error_);
        }
        int status = Connect(Socket_.Fd(), Dest_);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }
        Socket_.SetRemoteAddress(std::move(Dest_));
        return std::error_code{};
    }

} // namespace NAsync
