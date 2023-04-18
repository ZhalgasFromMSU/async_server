#include <net/accept_connect_awiatable.hpp>

#include <netdb.h>
#include <sys/socket.h>
#include <cstring>

namespace NAsync {

    namespace {
        TSocket::TAddr FromSockaddr(const sockaddr& addr) {
            if (addr.sa_family == AF_INET) {
                const auto& sockAddr = static_cast<const sockaddr_in*>((const void*)&addr);
                return std::make_pair(TIPv4Address{sockAddr->sin_addr}, sockAddr->sin_port);
            } else if (addr.sa_family == AF_INET6) {
                const auto& sockAddr = static_cast<const sockaddr_in6*>((const void*)&addr);
                return std::make_pair(TIPv6Address{sockAddr->sin6_addr}, sockAddr->sin6_port);
            } else {
                VERIFY(false);
            }
        }

        int Connect(int sockFd, const TSocket::TAddr& addr) {
            int status;
            if (auto addrv4 = std::get_if<std::pair<TIPv4Address, uint16_t>>(&addr)) {
                sockaddr_in addrIn;
                memset(&addrIn, 0, sizeof(addrIn));
                addrIn.sin_addr = addrv4->first.Raw();
                addrIn.sin_family = AF_INET;
                addrIn.sin_port = addrv4->second;
                status = connect(sockFd, static_cast<const sockaddr*>((const void*)&addrIn), sizeof(addrIn));
            } else if (auto addrv6 = std::get_if<std::pair<TIPv6Address, uint16_t>>(&addr)) {
                sockaddr_in6 addrIn;
                memset(&addrIn, 0, sizeof(addrIn));
                addrIn.sin6_addr = addrv6->first.Raw();
                addrIn.sin6_family = AF_INET6;
                addrIn.sin6_port = addrv6->second;
                status = connect(sockFd, static_cast<const sockaddr*>((const void*)&addrIn), sizeof(addrIn));
            } else {
                VERIFY(false);
            }

            return status;
        }
    }

    // Accept
    bool TAcceptAwaitable::await_ready() noexcept {
        sockaddr addr;
        socklen_t size;
        int sockFd = accept4(Socket_.Fd(), &addr, &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            if (errno & (EAGAIN || EWOULDBLOCK)) {
                return false;
            }
            NewSocket_.emplace(std::error_code{errno, std::system_category()});
        } else {
            NewSocket_.emplace(TSocket{sockFd, TSocket::EType::kTcp, FromSockaddr(addr)});
        }
        return true;
    }

    void TAcceptAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        if (ThreadPool) {
            Epoll->WatchForRead(Socket_, [this, handle] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->WatchForRead(Socket_, handle);
        }
    }

    TResult<TSocket> TAcceptAwaitable::await_resume() noexcept {
        if (NewSocket_) {
            return std::move(*NewSocket_);
        }

        sockaddr addr;
        socklen_t size;
        int sockFd = accept4(Socket_.Fd(), &addr, &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd, TSocket::EType::kTcp, FromSockaddr(addr)};
    }

    // Connect
    bool TConnectAwaitable::await_ready() noexcept {
        int status = Connect(Socket_.Fd(), RemoteSock_);
        if (status == -1) {
            if (errno & (EINPROGRESS | EAGAIN)) {
                return false;
            }
            ConnectError_ = std::error_code{errno, std::system_category()};
        } else {
            ConnectError_ = std::error_code{};
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
        if (ConnectError_) {
            return std::move(*ConnectError_);
        }
        int status = Connect(Socket_.Fd(), RemoteSock_);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return std::error_code{};
    }

} // namespace NAsync