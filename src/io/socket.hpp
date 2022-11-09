#pragma once

#include "io_base.hpp"

namespace NAsync {

    class TConnectionManager {
    public:
        struct TSocketOptions {
            bool Localhost;  // AF_LOCAL
            bool UseIpV6;
        };

        TConnectionManager(TSocketOptions = {}) noexcept;

    private:
        int Fd_;
    };

} // namespace NAsync