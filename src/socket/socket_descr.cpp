#include <socket/socket_descr.hpp>
#include <socket/socket.hpp>
#include <util/result.hpp>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>

namespace NAsync {

    // TIpAndPort
    TIpAndPort::TIpAndPort(EDomain domain, const sockaddr& sockAddr) noexcept {
        if (domain == EDomain::kUnix) {
            Ip = static_cast<const sockaddr_un*>((const void*)&sockAddr)->sun_path;
        } else {
            char buffer[std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
            int sockDomain;
            const void* src;
            if (domain == EDomain::kIPv4) {
                auto addr = static_cast<const sockaddr_in*>((const void*)&sockAddr);
                Port = ntohs(addr->sin_port);
                src = &addr->sin_addr;
                sockDomain = AF_INET;
            } else {
                auto addr = static_cast<const sockaddr_in6*>((const void*)&sockAddr);
                Port = ntohs(addr->sin6_port);
                src = &addr->sin6_addr;
                sockDomain = AF_INET6;
            }

            const char* str = inet_ntop(sockDomain, src, buffer, sizeof(buffer));
            VERIFY_SYSCALL(str != nullptr);
            Ip = str;
        }
    }
    // ~TIpAndPort

    // TSockDescr
    TSockDescr::TSockDescr(TSockDescr&&) noexcept = default;
    TSockDescr& TSockDescr::operator=(TSockDescr&&) noexcept = default;
    TSockDescr::~TSockDescr() = default;

    TSockDescr::TSockDescr(EDomain domain, ESockType type, TIpAndPort ipAndPort) noexcept
        : Domain_{domain}
        , Type_{type}
        , IpAndPort_{std::move(ipAndPort)}
    {}

    TSockDescr::TSockDescr(EDomain domain, ESockType type, const sockaddr& sockAddr) noexcept
        : Domain_{domain}
        , Type_{type}
        , IpAndPort_{Domain_, sockAddr}
    {}

    TSockDescr::TSockDescr(addrinfo info) noexcept
        : AddrInfo_{std::make_unique<addrinfo>(std::move(info))}
        , Domain_{ToDomain(AddrInfo_->ai_family)}
        , Type_{ToSockType(AddrInfo_->ai_socktype)}
        , IpAndPort_{Domain_, *AddrInfo_->ai_addr}
    {}

    TResult<TSocket> TSockDescr::CreateSocket() && noexcept {
        int sockFd = socket(AddrInfo_->ai_family, AddrInfo_->ai_socktype | SOCK_NONBLOCK, AddrInfo_->ai_protocol);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd, std::move(*this)};
    }
    // ~TSockDescr

    // Helper functions
    int FromDomain(EDomain domain) noexcept {
        if (domain == EDomain::kIPv4) {
            return AF_INET;
        } else if (domain == EDomain::kIPv6) {
            return AF_INET6;
        } else if (domain == EDomain::kUnix) {
            return AF_UNIX;
        }
        VERIFY(false);
    }

    EDomain ToDomain(int domain) noexcept {
        if (domain == AF_INET) {
            return EDomain::kIPv4;
        } else if (domain == AF_INET6) {
            return EDomain::kIPv6;
        } else if (domain == AF_UNIX) {
            return EDomain::kUnix;
        }
        VERIFY(false);
    }

    int FromSockType(ESockType type) noexcept {
        if (type == ESockType::kTcp) {
            return SOCK_STREAM;
        } else if (type == ESockType::kUdp) {
            return SOCK_DGRAM;
        } else if (type == ESockType::kRaw) {
            return SOCK_RAW;
        }
        VERIFY(false);
    }

    ESockType ToSockType(int type) noexcept {
        if (type == SOCK_STREAM) {
            return ESockType::kTcp;
        } else if (type == SOCK_DGRAM) {
            return ESockType::kUdp;
        } else if (type == SOCK_RAW){
            return ESockType::kRaw;
        }
        VERIFY(false);
    }

} // namespace NAsync