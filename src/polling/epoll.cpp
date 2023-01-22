#include <polling/epoll.hpp>
#include <util/result.hpp>

#include <sys/epoll.h>
#include <sys/unistd.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <cstring>

#include <system_error>
#include <future>

namespace NAsync {

    namespace {
        enum class EEpollMode {
            kRead,
            kWrite,
        };

        int CreateEpollFd() noexcept {
            int status = epoll_create1(0);
            VERIFY_SYSCALL(status >= 0);
            return status;
        }

        int AddFdToEpoll(int epollFd, int fdToWatch, EEpollMode mode) noexcept {
            epoll_event eventToAdd;
            memset(&eventToAdd, 0, sizeof(eventToAdd));
            eventToAdd.data.fd = fdToWatch;
            if (mode == EEpollMode::kRead) {
                eventToAdd.events = EPOLLIN | EPOLLET | EPOLLONESHOT; // not sure about epolloneshot, maybe do it under flag
            } else { // EEpollMode::kWrite
                eventToAdd.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
            }
            return epoll_ctl(epollFd, EPOLL_CTL_ADD, fdToWatch, &eventToAdd);
        }
    }

    TEpoll::TEpoll() noexcept
        : EpollFd_{CreateEpollFd()}
        , EpollStopped_{false}
        , EpollBackgroundThread_{&NAsync::TEpoll::PollLoop, this}
    {
        VERIFY_SYSCALL(AddFdToEpoll(EpollFd_.Fd(), EventFd_.Fd(), EEpollMode::kRead) >= 0);
    }

    TEpoll::~TEpoll() noexcept {
        EpollStopped_ = true;
        EventFd_.Set();
        EpollBackgroundThread_.join();
    }

    std::error_code TEpoll::WatchForRead(int fd, TCallback callback) noexcept {
        std::scoped_lock lock{CallbacksMutex_};
        auto it = Callbacks_.insert(Callbacks_.end(), std::move(callback));
        FdIteratorMapping_[fd] = it;
        if (AddFdToEpoll(EpollFd_.Fd(), fd, EEpollMode::kRead) < 0) { // epoll is thread safe
            return std::error_code{errno, std::system_category()};
        }
        return {};
    }


    std::error_code TEpoll::WatchForWrite(int fd, TCallback callback) noexcept {
        std::scoped_lock lock{CallbacksMutex_};
        auto it = Callbacks_.insert(Callbacks_.end(), std::move(callback));
        FdIteratorMapping_[fd] = it;
        if (AddFdToEpoll(EpollFd_.Fd(), fd, EEpollMode::kWrite) < 0) { // epoll is thread safe
            return std::error_code{errno, std::system_category()};
        }
        return {};
    }

    void TEpoll::PollLoop() noexcept {
        constexpr static size_t epollBuffSize = 1024;
        epoll_event buffer[epollBuffSize];
        while (true) {
            int status = epoll_wait(EpollFd_.Fd(), buffer, epollBuffSize, -1);
            VERIFY_SYSCALL(status >= 0);
            if (EpollStopped_) {
                break;
            }
            std::scoped_lock lock{CallbacksMutex_};
            for (int i = 0; i < status; ++i) {
                auto callbackIt = FdIteratorMapping_.at(buffer[i].data.fd);
                (*callbackIt)();
                FdIteratorMapping_.erase(buffer[i].data.fd);
                Callbacks_.erase(callbackIt);
            }
        }
    }

} // namespace NAsync