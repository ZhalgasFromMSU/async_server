#include "polling.hpp"

#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <unordered_map>
#include <exception>
#include <iostream>

namespace NAsync {

    // TEpoll
    TEpoll::TEpoll() noexcept
        : EpollFd_(epoll_create1(0))
    {}

    TEpoll::~TEpoll() noexcept {
        close(EpollFd_);
    }

    std::vector<TcIoPtr> TEpoll::WaitReadyIos(int timeoutInMs) const noexcept {
        epoll_event epollBuffer[EpollBufSize_];
        int count = epoll_wait(EpollFd_, epollBuffer, EpollBufSize_, timeoutInMs);
        if (count < 0) {
            std::terminate();
        }

        std::vector<TcIoPtr> readyIos;
        readyIos.reserve(count);
        for (int i = 0; i < count; ++i) {
            readyIos.push_back(*static_cast<TcIoPtr*>(epollBuffer[i].data.ptr));
        }
        return readyIos;
    }

    void TEpoll::AddToWatchlist(TcIoPtr io, TWatchlistOptions options) noexcept {
        int fd = io->GetFd();

        epoll_event eventToAdd;
        eventToAdd.data.ptr = &io;
        // eventToAdd.data.fd = fd;
        eventToAdd.events = 0;

        if (options.WaitForRead) {
            eventToAdd.events |= EPOLLIN;
        }
        if (options.WaitForWrite) {
            eventToAdd.events |= EPOLLOUT;
        }
        if (options.EdgeTriggered) {
            eventToAdd.events |= EPOLLET;
        }

        int status = epoll_ctl(EpollFd_, EPOLL_CTL_ADD, io->GetFd(), &eventToAdd);
        if (status < 0) {
            std::terminate();
        }

        FdObjectMapping_[fd] = io;
    }

    void TEpoll::RemoveFromWatchlist(TcIoPtr io) noexcept {
        int fd = io->GetFd();
        epoll_ctl(EpollFd_, EPOLL_CTL_DEL, fd, nullptr);
        FdObjectMapping_.erase(fd);
    }

    TcIoPtr TEpoll::GetObject(int fd) const noexcept {
        return FdObjectMapping_.at(fd);
    }

    // TPoll
    class TPoll::TFdContainer {
    public:
        TFdContainer() noexcept {
            FdVector_.reserve(MaxObservableSize_);
            IndexMapping_.reserve(MaxObservableSize_);
        }

        std::vector<pollfd>& Get() noexcept {
            return FdVector_;
        }

        TcIoPtr GetIoPtr(int fd) noexcept {
            return ObjectMapping_.at(fd);
        }

        void AddIoObject(TcIoPtr ioObject, TWatchlistOptions options) noexcept {
            if (FdVector_.size() == MaxObservableSize_) {
                std::terminate();
            }
            int fd = ioObject->GetFd();
            pollfd eventToAdd;
            eventToAdd.fd = fd;
            eventToAdd.events = 0;
            if (options.WaitForRead) {
                eventToAdd.events |= POLLIN;
            }
            if (options.WaitForWrite) {
                eventToAdd.events |= POLLOUT;
            }
            FdVector_.emplace_back(std::move(eventToAdd));
            IndexMapping_[fd] = FdVector_.size() - 1;
            ObjectMapping_[fd] = ioObject;
        }

        void RemoveIoObject(TcIoPtr ioObject) noexcept {
            int fd = ioObject->GetFd();
            size_t index = IndexMapping_.at(fd);
            std::swap(FdVector_[index], FdVector_.back());
            FdVector_.pop_back();
            IndexMapping_.erase(fd);
            ObjectMapping_.erase(fd);
        }

    private:
        static constexpr size_t MaxObservableSize_ = 1000;

        std::vector<pollfd> FdVector_;
        std::unordered_map<int, size_t> IndexMapping_; // fd -> index in FdVector_
        std::unordered_map<int, TcIoPtr> ObjectMapping_; // fd -> ioObect
    };

    TPoll::TPoll() noexcept {
        FdContainerPtr_ = new TFdContainer();
    }

    TPoll::~TPoll() noexcept {
        delete FdContainerPtr_;
    }

    std::vector<TcIoPtr> TPoll::WaitReadyIos(int timeoutInMs) const noexcept {
        int count = poll(FdContainerPtr_->Get().data(), FdContainerPtr_->Get().size(), timeoutInMs);
        if (count < 0) {
            std::terminate();
        }

        std::vector<TcIoPtr> readyIos;
        readyIos.reserve(count);
        for (const auto& fdInfo : FdContainerPtr_->Get()) {
            if (fdInfo.revents & (POLLIN | POLLOUT | POLLHUP | POLLWRNORM)) {
                readyIos.push_back(FdContainerPtr_->GetIoPtr(fdInfo.fd));
            } else if (fdInfo.revents != 0) {
                std::terminate();
            }
        }
        return readyIos;
    }

    void TPoll::AddToWatchlist(TcIoPtr io, TWatchlistOptions options) noexcept {
        FdContainerPtr_->AddIoObject(io, options);
    }

    void TPoll::RemoveFromWatchlist(TcIoPtr io) noexcept {
        FdContainerPtr_->RemoveIoObject(io);
    }

} // namespace NAsync