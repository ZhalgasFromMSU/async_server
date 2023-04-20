
#include "converter.hpp"
#include <net/socket.hpp>
#include <net/accept_connect_awiatable.hpp>
#include <system_error>

namespace NAsync {

    // TListeningSocket
    TResult<TListeningSocket> TListeningSocket::Create(TSocketAddress address, int queueSize) noexcept {
        NPrivate::TConverter cv{address};

        int sockFd = socket(cv.Domain(), SOCK_STREAM, 0);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }

        int bindRes = bind(sockFd, cv.Raw(), cv.Size());
        if (bindRes == -1) {
            return std::error_code{errno, std::system_category()};
        }

        NPrivate::TConverter cv2;
        int gsnRes = getsockname(sockFd, cv2.MutRaw(), cv2.MutSize());
        if (gsnRes == -1) {
            return std::error_code{errno, std::system_category()};
        }

        TSocketAddress actualAddress = cv2.FromRawStorage();

        int listenRes = listen(sockFd, queueSize);
        if (listenRes == -1) {
            return std::error_code{errno, std::system_category()};
        }

        return TListeningSocket{sockFd, std::move(actualAddress)};
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
        Remote_.emplace(std::move(dest));
        return TConnectAwaitable{*this};
    }

} // namespace NAsync
