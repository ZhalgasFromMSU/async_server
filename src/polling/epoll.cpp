#include <polling/epoll.hpp>

#include <mutex>

#include <sys/epoll.h>
#include <cstring>

namespace NAsync {

    namespace {
        std::error_code AddFdToEpoll(int epollFd, int fd, TEpoll::EMode mode) noexcept {
            epoll_event event;
            memset(&event, 0, sizeof(event));
            event.data.fd = fd;
            switch (mode) {
                case TEpoll::EMode::kRead:
                    event.events = EPOLLIN;
                    break;
                case TEpoll::EMode::kWrite:
                    event.events = EPOLLOUT;
                    break;
                default:
                    VERIFY(false);
            }

            event.events |= EPOLLET | EPOLLONESHOT;
            int status = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
            if (status == -1) {
                return std::error_code{errno, std::system_category()};
            }
            return std::error_code{};
        }
    } // namespace

    // TEpoll
    TEpoll::TEpoll() noexcept
        : TIoObject{epoll_create1(0)}
    {
        VERIFY_EC(Watch(EventFd_, []{}, TEpoll::EMode::kRead));
    }

    TEpoll::~TEpoll() {
        if (!Stopped_) {
            Stop();
        }
    }

    void TEpoll::Start() noexcept {
        Worker_ = std::thread {[this] {
            static constexpr size_t bSize = 1024;
            epoll_event buffer[bSize];
            while (!Stopped_) {
                int numReady = epoll_wait(Fd(), buffer, bSize, -1 /* timeout */);
                VERIFY_SYSCALL(numReady != -1);

                std::scoped_lock lock{Mutex_};
                for (int i = 0; i < numReady; ++i) {
                    auto node = Callbacks_.extract(buffer[i].data.fd);
                    VERIFY(!node.empty());
                    node.mapped()->Execute();
                }
            }
        }};
    }

    void TEpoll::Stop() noexcept {
        Stopped_ = true;
        EventFd_.Set();
        Worker_.join();

        std::scoped_lock lock{Mutex_};
        while (!Callbacks_.empty()) {
            auto node = Callbacks_.extract(Callbacks_.begin());
            node.mapped()->Execute();
        }
    }

    std::error_code TEpoll::Watch(const TIoObject& io, std::unique_ptr<ITask> callback, TEpoll::EMode mode) noexcept {
        std::scoped_lock lock{Mutex_};
        if (Stopped_) {
            return std::error_code{EBADF, std::system_category()};
        }
        Callbacks_[io.Fd()] = std::move(callback);
        return AddFdToEpoll(Fd(), io.Fd(), mode);
    }

} // namespace NAsync
