#include <socket/socket_descr.hpp>
#include <socket/socket.hpp>
#include <util/result.hpp>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>

namespace NAsync {

    // TSockDescr
    TSockDescr::TSockDescr(TSockDescr&&) noexcept = default;
    TSockDescr& TSockDescr::operator=(TSockDescr&&) noexcept = default;
    TSockDescr::~TSockDescr() = default;

    TSockDescr::TSockDescr(addrinfo info) noexcept
        : AddrInfo_{std::make_unique<addrinfo>(std::move(info))}
        , Domain_{ToDomain(AddrInfo_->ai_family)}
        , Type_{ToSockType(AddrInfo_->ai_socktype)}
    {
        if (Domain_ == EDomain::kUnix) {
            StrAddr_ = static_cast<sockaddr_un*>((void*)AddrInfo_->ai_addr)->sun_path;
        } else {
            char buffer[std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
            int domain;
            const void* src;
            if (Domain_ == EDomain::kIPv4) {
                sockaddr_in* addr = static_cast<sockaddr_in*>((void*)AddrInfo_->ai_addr);
                Port_ = ntohs(addr->sin_port);
                src = &addr->sin_addr;
                domain = AF_INET;
            } else {
                sockaddr_in6* addr = static_cast<sockaddr_in6*>((void*)AddrInfo_->ai_addr);
                Port_ = ntohs(addr->sin6_port);
                src = &addr->sin6_addr;
                domain = AF_INET6;
            }

            const char* str = inet_ntop(domain, src, buffer, sizeof(buffer));
            VERIFY_SYSCALL(str != nullptr);
            StrAddr_ = str;
        }
    }

    TSocket TSockDescr::CreateSocket() && noexcept {
        return TSocket{std::move(*this)};
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