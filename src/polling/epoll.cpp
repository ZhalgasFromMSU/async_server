#include <polling/epoll.hpp>
#include <util/result.hpp>

#include <sys/epoll.h>
#include <sys/unistd.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <cstring>

#include <system_error>
#include <future>
#include <iostream>

namespace NAsync {

    namespace {
        enum class EEpollMode {
            kRead,
            kWrite,
        };

        int AddFdToEpoll(int epollFd, int fdToWatch, void* callbackAddr, EEpollMode mode) {
            epoll_event eventToAdd;
            memset(&eventToAdd, 0, sizeof(eventToAdd));
            eventToAdd.data.ptr = callbackAddr;
            if (mode == EEpollMode::kRead) {
                eventToAdd.events = EPOLLIN | EPOLLET;
            } else { // EEpollMode::kWrite
                eventToAdd.events = EPOLLOUT | EPOLLET;
            }
            return epoll_ctl(epollFd, EPOLL_CTL_ADD, fdToWatch, &eventToAdd);
        }
    }

    TEpoll::TEpoll() noexcept
        : EpollFd_{epoll_create1(0)}
        , EventFd_{eventfd(0, EFD_NONBLOCK)}
        , EpollFuture_{std::async(&TEpoll::PollFunc, this)}
    {
        VERIFY_EC(WatchForRead(EventFd_.Fd(), std::bind(&TEpoll::EventFdCallback, this)));
    }

    TEpoll::~TEpoll() noexcept {
        ShouldFinish_ = true;
        uint64_t numToWrite = 1;
        VERIFY_RESULT(EventFd_.Write(&numToWrite, sizeof(numToWrite)));
        EpollFuture_.wait_for(std::chrono::seconds(1));
        VERIFY_SYSCALL(close(EpollFd_) >= 0);
    }

    void TEpoll::EventFdCallback() noexcept {
        uint64_t val;
        VERIFY_RESULT(EventFd_.Read(&val, sizeof(val)));
        VERIFY_EC(WatchForRead(EventFd_.Fd(), std::bind(&TEpoll::EventFdCallback, this)));
    }

    size_t TEpoll::Size() const noexcept {
        std::scoped_lock lock{EpollMutex_};
        return Callbacks_.size() - 1;
    }

    std::error_code TEpoll::WatchForRead(int fd, TCallback callback) noexcept {
        std::scoped_lock lock{EpollMutex_};
        Callbacks_.push_back({fd, std::move(callback)});
        if (AddFdToEpoll(EpollFd_, fd, Callbacks_.tail_pointer(), EEpollMode::kRead) < 0) {
            return std::error_code{errno, std::system_category()};
        }
        return {};
    }


    std::error_code TEpoll::WatchForWrite(int fd, TCallback callback) noexcept {
        std::scoped_lock lock{EpollMutex_};
        Callbacks_.push_back({fd, std::move(callback)});
        if (AddFdToEpoll(EpollFd_, fd, Callbacks_.tail_pointer(), EEpollMode::kWrite) < 0) {
            return std::error_code{errno, std::system_category()};
        }
        return {};
    }

    void TEpoll::PollFunc() noexcept {
        epoll_event buffer[EpollBuffSize_];
        while (!ShouldFinish_) {
            int count = epoll_wait(EpollFd_, buffer, EpollBuffSize_, -1);
            std::scoped_lock lock{EpollMutex_};
            VERIFY_SYSCALL(count >= 0);
            for (int i = 0; i < count; ++i) {
                auto callbackPtr = static_cast<TList<std::pair<int, TCallback>>::TNode*>(buffer[i].data.ptr);
                auto& [fd, callback] = callbackPtr->Val();
                VERIFY_SYSCALL(epoll_ctl(EpollFd_, EPOLL_CTL_DEL, fd, nullptr) >= 0);
                callback();
                Callbacks_.erase(callbackPtr);
            }
        }
    }

} // namespace NAsync