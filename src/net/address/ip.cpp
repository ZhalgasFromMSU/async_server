#include <net/address/ip.hpp>

#include <netinet/ip.h>
#include <arpa/inet.h>

namespace NAsync {

    // TIPv4Address
    TIPv4Address::TIPv4Address(in_addr addr) noexcept
        : Raw_ {new in_addr{std::move(addr)}}
    {}

    std::string TIPv4Address::ToString() const noexcept {
        char buffer[INET_ADDRSTRLEN];
        const char* str = inet_ntop(AF_INET, Raw_.get(), buffer, sizeof(buffer));
        VERIFY_SYSCALL(str != nullptr);
        return std::string{str};
    }

    TResult<TIPv4Address> TIPv4Address::FromString(const std::string& s) noexcept {
        in_addr addr;
        int status = inet_aton(s.c_str(), &addr);
        if (status == 1) {
            return TIPv4Address{std::move(addr)};
        }

        return std::error_code{EAFNOSUPPORT, std::system_category()};
    }

    // TIPv6Address
    TIPv6Address::TIPv6Address(in6_addr addr) noexcept
        : Raw_ {new in6_addr{std::move(addr)}}
    {}

    std::string TIPv6Address::ToString() const noexcept {
        char buffer[INET6_ADDRSTRLEN];
        const char* str = inet_ntop(AF_INET6, Raw_.get(), buffer, sizeof(buffer));
        VERIFY_SYSCALL(str != nullptr);
        return std::string{str};
    }

    TResult<TIPv6Address> TIPv6Address::FromString(const std::string& s) noexcept {
        in6_addr addr;
        int status = inet_pton(AF_INET6, s.c_str(), &addr);
        if (status == 1) {
            return TIPv6Address{std::move(addr)};
        }

        return std::error_code{errno, std::system_category()};
    }

} // namespace NAsync
