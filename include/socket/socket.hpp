#pragma once

#include "resolve.hpp"

#include <memory>

namespace NAsync {
    enum class EDomain {
        kIpV4,
        kIpV6,
        kUnix,
    };

    enum class ESockType {
        kUdp,
        kTcp,
        kRaw,
    };

    class TSocket {
    public:
        explicit TSocket(TSocketAddress addr) noexcept;

    private:
        TSocketAddress Addr_;
        int Fd_ = 0;
    };

} // namespace NAsync