#pragma once

#include <util/result.hpp>

#include <memory>

struct addrinfo;
struct sockaddr;

namespace NAsync {

    enum class EDomain {
        kIPv4,
        kIPv6,
        kUnix,
    };

    enum class ESockType {
        kTcp,
        kUdp,
        kRaw,
    };

    struct TAddress {
        static TAddress FromSockaddr(EDomain domain, const sockaddr& sockAddr) noexcept;

        EDomain Domain;
        std::string Ip;
        uint16_t Port = 0;
    };

    class TSocket;

    class TSockDescr {
    public:
        TSockDescr(TSockDescr&&) noexcept;
        TSockDescr& operator=(TSockDescr&&) noexcept;
        ~TSockDescr();

        TSockDescr(EDomain domain, ESockType type) noexcept;
        TSockDescr(EDomain domain, ESockType type, TAddress address) noexcept;
        TSockDescr(EDomain domain, ESockType type, const sockaddr& sockAddr) noexcept;

        inline const addrinfo& AddrInfo() const noexcept {
            return *AddrInfo_;
        }

        inline EDomain Domain() const noexcept {
            return Address_.Domain;
        }

        inline ESockType Type() const noexcept {
            return Type_;
        }

        inline const std::string& StrAddr() const noexcept {
            return Address_.Ip;
        }

        inline uint16_t Port() const noexcept {
            return Address_.Port;
        }

        TResult<TSocket> CreateSocket() && noexcept;

    private:
        friend TSocket;
        friend class TResolver;

        TSockDescr(addrinfo addrinfo) noexcept;

        std::unique_ptr<addrinfo> AddrInfo_;
        ESockType Type_;
        TAddress Address_;
    };

    // Helper functions
    int FromDomain(EDomain domain) noexcept; // to c socket domain (AF_INET, AF_INET6, AF_UNIX)
    EDomain ToDomain(int domain) noexcept;
    int FromSockType(ESockType type) noexcept; // SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
    ESockType ToSockType(int type) noexcept;
} // namespace NAsync