#pragma once

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

    int FromDomain(EDomain domain) noexcept; // to c socket domain (AF_INET, AF_INET6, AF_UNIX)
    EDomain ToDomain(int domain) noexcept;

    int FromSockType(ESockType type) noexcept; // SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
    ESockType ToSockType(int type) noexcept;

} // namespace NAsync
