#pragma once

#include <vector>
#include <memory>

#include "base.hpp"  // TODO

namespace NAsync {

    class TPoller {
    public:
        // Blocks thread
        std::vector<const IIoObject*> WaitAvailableIos() const noexcept;

        // If edgeTriggered flag specified, then epoll will notify only on new data available
        // E.g. if some data left in channel, then it won't trigger epoll again
        void AddToWatchlist(const IIoObject* ioObject, bool edgeTriggered = false) const noexcept;
        void RemoveFromWatchlist(const IIoObject* ioObject) const noexcept;

    private:
        class TEpollImpl;
        std::unique_ptr<TEpollImpl> EpollPtr_;
    };

} // NAsync