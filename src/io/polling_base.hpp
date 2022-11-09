#pragma once

#include "io_base.hpp"

#include <vector>

namespace NAsync {

    struct TWatchlistOptions {
        bool WaitForRead = false;
        bool WaitForWrite = false;
        bool EdgeTriggered = false; // Only notifies on new data. User should always read until block or EOF. Only applicable to epoll
    };

    class IPoller {
    public:
        virtual ~IPoller() = default;
        virtual std::vector<TcIoPtr> WaitReadyIos(int timeoutInMs) const noexcept = 0;
        std::vector<TcIoPtr> WaitReadyIos() const noexcept { return WaitReadyIos(-1); }

        virtual void AddToWatchlist(TcIoPtr ioObject, TWatchlistOptions options) noexcept = 0;
        virtual void RemoveFromWatchlist(TcIoPtr ioObject) noexcept = 0;
    };

}