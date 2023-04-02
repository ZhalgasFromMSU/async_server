#pragma once

#include <memory>

struct addrinfo;

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

    class TSocket;

    class TSockDescr {
    public:
        TSockDescr(TSockDescr&&) noexcept;
        TSockDescr& operator=(TSockDescr&&) noexcept;
        ~TSockDescr();

        inline EDomain Domain() const noexcept {
            return Domain_;
        }

        inline ESockType Type() const noexcept {
            return Type_;
        }

        inline uint16_t Port() const noexcept {
            return Port_;
        }

        const std::string& StrAddr() const noexcept {
            return StrAddr_;
        }

        TSocket CreateSocket() && noexcept;

    private:
        TSockDescr(addrinfo addrinfo) noexcept;

        friend TSocket;
        friend class TResolver;

        std::unique_ptr<addrinfo> AddrInfo_;
        EDomain Domain_;
        ESockType Type_;
        std::string StrAddr_;
        uint16_t Port_ = 0;
    };

    // Helper functions
    int FromDomain(EDomain domain) noexcept; // to c socket domain (AF_INET, AF_INET6, AF_UNIX)
    EDomain ToDomain(int domain) noexcept;
    int FromSockType(ESockType type) noexcept; // SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
    ESockType ToSockType(int type) noexcept;
} // namespace NAsync