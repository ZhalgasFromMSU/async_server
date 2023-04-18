#include <net/socket.hpp>
#include <net/accept_connect_awiatable.hpp>

#include <sys/socket.h>
#include <netdb.h>
#include <cstring>

namespace NAsync {

    TSocket::TSocket(int fd, EType type, TAddr addr) noexcept
        : TIoObject{fd}
        , Type_{type}
        , Address_{std::move(addr)}
    {}

    TResult<TSocket> TSocket::CreateListeningSocket(EType type, TAddr address) noexcept {
        int domain = AF_INET6;
        if (std::holds_alternative<std::pair<TIPv4Address, uint16_t>>(address)) {
            domain = AF_INET;
        }

        int sockType = SOCK_STREAM;
        if (type == EType::kUdp) {
            sockType = SOCK_DGRAM;
        }

        int sockFd = socket(domain, sockType | SOCK_NONBLOCK, 0);
        if (sockFd < 0) {
            return std::error_code{errno, std::system_category()};
        }

        if (domain == AF_INET6) {
            const auto& socketAddress = std::get<std::pair<TIPv6Address, uint16_t>>(address);
            sockaddr_in6 rawAddr;
            memset(&rawAddr, 0, sizeof(rawAddr));
            rawAddr.sin6_addr = socketAddress.first.Raw();
            rawAddr.sin6_family = AF_INET6;
            rawAddr.sin6_port = socketAddress.second;

            int status = bind(sockFd, static_cast<const sockaddr*>((const void*)&rawAddr), sizeof(rawAddr));
            if (status < 0) {
                return std::error_code{errno, std::system_category()};
            }
        } else if (domain == AF_INET) {
            const auto& socketAddress = std::get<std::pair<TIPv4Address, uint16_t>>(address);
            sockaddr_in rawAddr;
            memset(&rawAddr, 0, sizeof(rawAddr));
            rawAddr.sin_addr = socketAddress.first.Raw();
            rawAddr.sin_family = AF_INET;
            rawAddr.sin_port = socketAddress.second;

            int status = bind(sockFd, static_cast<const sockaddr*>((const void*)&rawAddr), sizeof(rawAddr));
            if (status < 0) {
                return std::error_code{errno, std::system_category()};
            }
        }

        int status = listen(sockFd, ListenintQueueSize_);
        if (status < 0) {
            return std::error_code{errno, std::system_category()};
        }

        return TResult<TSocket>::Build(sockFd, type, std::move(address));
    }

    TAcceptAwaitable TSocket::Accept() const noexcept {
        return TAcceptAwaitable{*this};
    }

    TConnectAwaitable TSocket::Connect(const TSocket::TAddr& remoteAddr) const noexcept {
        return TConnectAwaitable{*this, remoteAddr};
    }

} // namespace NAsync