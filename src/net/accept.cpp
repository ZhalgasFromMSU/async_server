#include "helpers.hpp"
#include <net/accept_connect_awiatable.hpp>

namespace NAsync {

    namespace {

        TSocketAddress ToSocketAddress(const sockaddr& raw) {
            if (raw.sa_family == AF_INET) {
                const auto* rawV4 = NPrivate::PtrCast<sockaddr_in>(&raw);
                return TSocketAddress{
                    std::in_place_type<TIPv4SocketAddress>,
                    std::piecewise_construct,
                    std::forward_as_tuple(rawV4->sin_addr),
                    std::forward_as_tuple(rawV4->sin_port)
                };
            } else if (raw.sa_family == AF_INET6) {
                const auto* rawV6 = NPrivate::PtrCast<sockaddr_in6>(&raw);
                return TSocketAddress{
                    std::in_place_type<TIPv6SocketAddress>,
                    std::piecewise_construct,
                    std::forward_as_tuple(rawV6->sin6_addr),
                    std::forward_as_tuple(rawV6->sin6_port)
                };
            } else {
                VERIFY(false);
            }
        }

    } // namespace

    // TAcceptAwaitable
    bool TAcceptAwaitable::await_ready() noexcept {
        uint8_t raw[std::max(sizeof(sockaddr_in), sizeof(sockaddr_in6))];
        socklen_t size = sizeof(raw);
        int sockFd = accept4(Acceptor_.Fd(), NPrivate::PtrCast<sockaddr>(&raw), &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            if (errno & (EAGAIN | EWOULDBLOCK)) {
                return false;
            }
            Socket_.emplace(std::error_code{errno, std::system_category()});
        } else {
            VERIFY(size <= sizeof(raw));
            Socket_.emplace(TSocket{sockFd, ToSocketAddress(*NPrivate::PtrCast<sockaddr>(&raw))});
        }
        return true;
    }

    void TAcceptAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        if (ThreadPool) {
            Epoll->WatchForRead(Acceptor_, [this, handle] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->WatchForRead(Acceptor_, handle);
        }
    }

    TResult<TSocket> TAcceptAwaitable::await_resume() noexcept {
        if (Socket_) {
            return std::move(*Socket_);
        }

        sockaddr raw;
        memset(&raw, 0, sizeof(raw));
        socklen_t size = 0;
        int sockFd = accept4(Acceptor_.Fd(), &raw, &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd, ToSocketAddress(raw)};
    }

} // namespace NAsync
