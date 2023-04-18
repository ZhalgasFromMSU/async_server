#include <net/address/ip.hpp>

#include <netinet/ip.h>
#include <arpa/inet.h>

namespace NAsync {

    // TIPv4Address
    TIPv4Address::TIPv4Address(in_addr addr) noexcept
        : RawAddr_ {new in_addr{std::move(addr)}}
    {}

    TIPv4Address::TIPv4Address(TIPv4Address&&) noexcept = default;
    TIPv4Address& TIPv4Address::operator=(TIPv4Address&&) noexcept = default;
    TIPv4Address::~TIPv4Address() = default;

    const in_addr& TIPv4Address::Raw() const noexcept {
        return *RawAddr_;
    }

    std::string TIPv4Address::ToString() const noexcept {
        char buffer[INET_ADDRSTRLEN];
        const char* str = inet_ntop(AF_INET, RawAddr_.get(), buffer, sizeof(buffer));
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

    TIPv4Address TIPv4Address::Localhost() noexcept {
        return *FromString("127.0.0.1");
    }

    TIPv4Address TIPv4Address::Any() noexcept {
        return *FromString("0.0.0.0");
    }

    // TIPv6Address
    TIPv6Address::TIPv6Address(in6_addr addr) noexcept
        : RawAddr_ {new in6_addr{std::move(addr)}}
    {}

    TIPv6Address::TIPv6Address(TIPv6Address&&) noexcept = default;
    TIPv6Address& TIPv6Address::operator=(TIPv6Address&&) noexcept = default;

    TIPv6Address::~TIPv6Address() = default;

    const in6_addr& TIPv6Address::Raw() const noexcept {
        return *RawAddr_;
    }

    std::string TIPv6Address::ToString() const noexcept {
        char buffer[INET6_ADDRSTRLEN];
        const char* str = inet_ntop(AF_INET6, RawAddr_.get(), buffer, sizeof(buffer));
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

    TIPv6Address TIPv6Address::Localhost() noexcept {
        return *FromString("::1");
    }

    TIPv6Address TIPv6Address::Any() noexcept {
        return *FromString("::");
    }

} // namespace NAsync