#pragma once

#include "io_base.hpp"

#include <optional>

namespace NAsync {

    struct TSocketOptions {
        bool Localhost = false;  // AF_LOCAL
        bool IpV6 = false;
        bool UDP = false;  // TCP or UDP
    };

    class TServerManager;

    class TSocket: public IIoObject {
    public:
        ~TSocket() noexcept;
        int GetFd() const noexcept override;

        static std::optional<TSocket> Connect(TSocketOptions socketOptions = {}) noexcept;

    private:
        friend TServerManager;
        TSocket(int fd) noexcept; // make sure that created socket is either from TServerManager or returend by Connect

        int Fd_;
    };

    class TServerManager {
    public:
        TServerManager(int port, TSocketOptions = {}) noexcept;
        ~TServerManager() noexcept;

        // blocks thread
        // param specifies if resulting socket is going to be non blocking
        TSocket AcceptConnection(bool returnNonBlocking = true) const noexcept;

    private:
        static constexpr int ConnectionQueueSize_ = 10;

        int Fd_;
    };

} // namespace NAsync