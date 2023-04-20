#pragma once

#include <net/socket.hpp>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <cstring>

namespace NAsync::NPrivate {

    template<typename To, typename From>
    std::conditional_t<std::is_const_v<From>, const To, To>* PtrCast(From* ptr) {
        if constexpr (std::is_const_v<From>) {
            return static_cast<const To*>(static_cast<const void*>(ptr));
        } else {
            return static_cast<To*>(static_cast<void*>(ptr));
        }
    }

    // Converting
    inline void ConvertSockaddr(const TSocketAddress& addr, sockaddr_in* ret) {
        const auto& inAddr = std::get<TIPv4SocketAddress>(addr);
        memset(ret, 0, sizeof(sockaddr_in));
        ret->sin_family = AF_INET;
        ret->sin_addr = inAddr.first.Raw();
        ret->sin_port = inAddr.second;
    }

    inline void ConvertSockaddr(const TSocketAddress& addr, sockaddr_in6* ret) {
        const auto& in6Addr = std::get<TIPv6SocketAddress>(addr);
        memset(ret, 0, sizeof(sockaddr_in6));
        ret->sin6_family = AF_INET6;
        ret->sin6_addr = in6Addr.first.Raw();
        ret->sin6_port = in6Addr.second;
    }

} // namespace NAsync::NPrivate
