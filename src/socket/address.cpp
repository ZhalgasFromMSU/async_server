#include <socket/address.hpp>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#include <iostream>

namespace NAsync {

    namespace {
        // class TResolveCategory: public std::error_category {
        // public:
        //     const char* name() const noexcept override {
        //         return "resolve category";
        //     }

        //     std::string message(int aiErr) const override {
        //         return gai_strerror(aiErr);
        //     }
        // };

        // const TResolveCategory resolveCategory{};

        // class TIpV4Address: public IAddress {
        // public:
        //     TIpV4Address(in_addr addr) noexcept
        //         : Addr_{std::move(addr)}
        //     {}

        //     bool IsIPv6() const noexcept override {
        //         return false;
        //     }

        //     std::string ToString() const noexcept override {
        //         char buffer[INET_ADDRSTRLEN];
        //         const char* str = inet_ntop(AF_INET, &Addr_, buffer, INET_ADDRSTRLEN);
        //         VERIFY_SYSCALL(str != nullptr);
        //         return str;
        //     }

        //     std::pair<const void*, size_t> ToPtr() const noexcept override {
        //         return std::make_pair(&Addr_, sizeof(Addr_));
        //     }

        // private:
        //     in_addr Addr_;
        // };

        // class TIpV6Address: public IAddress {
        // public:
        //     TIpV6Address(in6_addr addr) noexcept
        //         : Addr_{std::move(addr)}
        //     {}

        //     bool IsIPv6() const noexcept override {
        //         return true;
        //     }

        //     std::string ToString() const noexcept override {
        //         char buffer[INET6_ADDRSTRLEN];
        //         const char* str = inet_ntop(AF_INET6, &Addr_, buffer, INET6_ADDRSTRLEN);
        //         VERIFY_SYSCALL(str != nullptr);
        //         return str;
        //     }

        //     std::pair<const void*, size_t> ToPtr() const noexcept override {
        //         return std::make_pair(&Addr_, sizeof(Addr_));
        //     }

        // private:
        //     in6_addr Addr_;
        // };

    }

    // const std::error_category& resolve_category() noexcept {
    //     return resolveCategory;
    // }

    // TResolveResult ResolveSync(const char* node, const char* service, EDomain domain) noexcept {
    //     TResolveResult::Type result;
    //     addrinfo* lookupResult;
    //     addrinfo hints;
    //     memset(&hints, 0, sizeof(hints));
    //     hints.ai_socktype = SOCK_STREAM;
    //     if (domain == EDomain::kIpV4) {
    //         hints.ai_family = AF_INET;
    //     } else if (domain == EDomain::kIpV6) {
    //         hints.ai_family = AF_INET6;
    //     }

    //     int status = getaddrinfo(node, service, &hints, &lookupResult);
    //     if (status != 0) {
    //         return std::error_code{status, resolve_category()};
    //     }

    //     for (auto ai = lookupResult; ai != nullptr; ai = ai->ai_next) {
    //         std::cerr << ai->ai_protocol << '\t' << ai->ai_socktype << '\t' << ai->ai_flags << std::endl;
    //         void* sockAddr = ai->ai_addr;
    //         if (ai->ai_family == AF_INET) {
    //             result.emplace_back(std::make_shared<TIpV4Address>(std::move(static_cast<sockaddr_in*>(sockAddr)->sin_addr)));
    //         } else { // AF_INET6
    //             result.emplace_back(std::make_shared<TIpV6Address>(std::move(static_cast<sockaddr_in6*>(sockAddr)->sin6_addr)));
    //         }
    //     }

    //     freeaddrinfo(lookupResult);
    //     return std::move(result);
    // }

    TIpAddress::TIpAddress(std::string ipAddress) noexcept
        : IpAddress_{std::move(ipAddress)}
    {}

    bool TIpAddress::IsIPv6() const noexcept {

    }

} // namespace NAsync
