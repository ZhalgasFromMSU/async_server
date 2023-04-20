
#include "helpers.hpp"
#include <net/socket.hpp>
#include <net/accept_connect_awiatable.hpp>
#include <system_error>

namespace NAsync {

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
            NPrivate::ConvertSockaddr(address, &raw);
            bindRes = bind(sockFd, NPrivate::PtrCast<sockaddr>(&raw), sizeof(raw));
        } else if (domain == AF_INET6) {
            sockaddr_in6 raw;
            NPrivate::ConvertSockaddr(address, &raw);
            bindRes = bind(sockFd, NPrivate::PtrCast<sockaddr>(&raw), sizeof(raw));
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
    template<>
    TResult<TSocket> TSocket::Create<TIPv4Address>(bool streamSocket) noexcept {
        int sockFd = socket(AF_INET, (streamSocket ? SOCK_STREAM : SOCK_DGRAM) | SOCK_NONBLOCK, 0);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd};
    }

    template<>
    TResult<TSocket> TSocket::Create<TIPv6Address>(bool streamSocket) noexcept {
        int sockFd = socket(AF_INET6, (streamSocket ? SOCK_STREAM : SOCK_DGRAM) | SOCK_NONBLOCK, 0);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd};
    }

    TConnectAwaitable TSocket::Connect(TSocketAddress dest) noexcept {
        return TConnectAwaitable{*this, std::move(dest)};
    }

} // namespace NAsync
