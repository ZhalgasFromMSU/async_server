#include "net/address/ip.hpp"
#include <net/socket.hpp>
#include <net/accept_connect_awiatable.hpp>

#include <sys/socket.h>
#include <netdb.h>
#include <cstring>

namespace NAsync {

    namespace {
        template<typename To, typename From>
        const To* ConstPtrCast(const From* ptr) {
            return static_cast<const To*>(static_cast<const void*>(ptr));
        }

        void ConvertSockaddr(const TSocketAddress& addr, sockaddr_in* ret) {
            const auto& inAddr = std::get<TIPv4SocketAddress>(addr);
            memset(ret, 0, sizeof(sockaddr_in));
            ret->sin_family = AF_INET;
            ret->sin_addr = inAddr.first.Raw();
            ret->sin_port = inAddr.second;
        }

        void ConvertSockaddr(const TSocketAddress& addr, sockaddr_in6* ret) {
            const auto& in6Addr = std::get<TIPv6SocketAddress>(addr);
            memset(ret, 0, sizeof(sockaddr_in6));
            ret->sin6_family = AF_INET6;
            ret->sin6_addr = in6Addr.first.Raw();
            ret->sin6_port = in6Addr.second;
        }

        TSocketAddress ToSocketAddress(const sockaddr& raw) {
            if (raw.sa_family == AF_INET) {
                const auto* rawV4 = ConstPtrCast<sockaddr_in>(&raw);
                return TSocketAddress{
                    std::in_place_type<TIPv4SocketAddress>,
                    std::piecewise_construct,
                    std::forward_as_tuple(rawV4->sin_addr),
                    std::forward_as_tuple(rawV4->sin_port)
                };
            } else if (raw.sa_family == AF_INET6) {
                const auto* rawV6 = ConstPtrCast<sockaddr_in6>(&raw);
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

        int Connect(int sockFd, const TSocketAddress& dest) {
            int status;
            if (std::holds_alternative<TIPv4SocketAddress>(dest)) {
                sockaddr_in raw;
                ConvertSockaddr(dest, &raw);
                status = connect(sockFd, ConstPtrCast<sockaddr>(&raw), sizeof(raw));
            } else if (std::holds_alternative<TIPv6SocketAddress>(dest)) {
                sockaddr_in6 raw;
                ConvertSockaddr(dest, &raw);
                status = connect(sockFd, ConstPtrCast<sockaddr>(&raw), sizeof(raw));
            } else {
                VERIFY(false);
            }

            return status;
        }
    } // namespace

    // TListeningSocket
    TResult<TListeningSocket> TListeningSocket::Create(TSocketAddress address, int queueSize) noexcept {
        int domain = AF_INET;
        if (std::holds_alternative<TIPv6SocketAddress>(address)) {
            domain = AF_INET6;
        }

        int sockFd = socket(domain, SOCK_STREAM, 0);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }

        int bindRes;
        if (domain == AF_INET) {
            sockaddr_in raw;
            ConvertSockaddr(address, &raw);
            bindRes = bind(sockFd, ConstPtrCast<sockaddr>(&raw), sizeof(raw));
        } else if (domain == AF_INET6) {
            sockaddr_in6 raw;
            ConvertSockaddr(address, &raw);
            bindRes = bind(sockFd, ConstPtrCast<sockaddr>(&raw), sizeof(raw));
        } else {
            VERIFY(false);
        }

        if (bindRes == -1) {
            return std::error_code{errno, std::system_category()};
        }

        int listenRes = listen(sockFd, queueSize);
        if (listenRes == -1) {
            return std::error_code{errno, std::system_category()};
        }

        return TListeningSocket{sockFd, std::move(address)};
    }

    TAcceptAwaitable TListeningSocket::Accept() const noexcept {
        return TAcceptAwaitable{*this};
    }

    // TSocket
    TConnectAwaitable TSocket::Connect(TSocketAddress dest) noexcept {
        return TConnectAwaitable{*this, std::move(dest)};
    }

    // TAcceptAwaitable
    bool TAcceptAwaitable::await_ready() noexcept {
        sockaddr raw;
        socklen_t size;
        int sockFd = accept4(Acceptor_.Fd(), &raw, &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            if (errno & (EAGAIN | EWOULDBLOCK)) {
                return false;
            }
            Socket_.emplace(std::error_code{errno, std::system_category()});
        } else {
            Socket_.emplace(TSocket{sockFd, ToSocketAddress(raw)});
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
        socklen_t size;
        int sockFd = accept4(Acceptor_.Fd(), &raw, &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd, ToSocketAddress(raw)};
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
