#pragma once

#include "polling_base.hpp"

#include <unordered_map>

namespace NAsync {

    class TEpoll: public IPoller {
    public:
        TEpoll() noexcept;
        ~TEpoll() noexcept;
        std::vector<TcIoPtr> WaitReadyIos(int timeoutInMs) const noexcept override;
        void AddToWatchlist(TcIoPtr ioObject, TWatchlistOptions options) noexcept override;
        void RemoveFromWatchlist(TcIoPtr ioObject) noexcept override;

        TcIoPtr GetObject(int fd) const noexcept;

    private:
        static constexpr size_t EpollBufSize_ = 1000;

        int EpollFd_;
        std::unordered_map<int, TcIoPtr> FdObjectMapping_; // fd -> ioObject
    };

    class TPoll: public IPoller {
    public:
        TPoll() noexcept;
        ~TPoll() noexcept;
        std::vector<TcIoPtr> WaitReadyIos(int timeoutInMs) const noexcept override;
        void AddToWatchlist(TcIoPtr ioObject, TWatchlistOptions options) noexcept override;
        void RemoveFromWatchlist(TcIoPtr ioObject) noexcept override;

    private:
        class TFdContainer;
        TFdContainer* FdContainerPtr_;
    };
} // NAsync