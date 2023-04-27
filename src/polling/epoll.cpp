#include <polling/epoll.hpp>

#include <sys/epoll.h>
#include <cstring>
#include <system_error>

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
        VERIFY_EC(Watch(TEpoll::EMode::kRead, EventFd_, []{}));
    }

    TEpoll::~TEpoll() {
        if (!Wg_.Waited()) {
            Stop();
        }
    }

    void TEpoll::Start() noexcept {
        Worker_ = std::thread {[this] {
            static constexpr size_t bSize = 1024;
            epoll_event buffer[bSize];
            while (!Wg_.Blocked()) {
                int numReady = epoll_wait(Fd(), buffer, bSize, -1 /* timeout */);
                VERIFY_SYSCALL(numReady != -1);

                std::scoped_lock lock{Mutex_};
                for (int i = 0; i < numReady; ++i) {
                    auto node = Callbacks_.extract(buffer[i].data.fd);
                    VERIFY(!node.empty());
                    node.mapped().Execute();
                    Wg_.Dec();
                }
            }
        }};
    }

    void TEpoll::Stop() noexcept {
        Wg_.Block();
        EventFd_.Set();
        Worker_.join();

        std::scoped_lock lock{Mutex_};
        while (!Callbacks_.empty()) {
            auto node = Callbacks_.extract(Callbacks_.begin());
            node.mapped().Execute();
            Wg_.Dec();
        }
    }

    std::error_code TEpoll::Watch(EMode mode, const TIoObject& io, TJob callback) noexcept {
        if (!Wg_.Inc()) {
            return std::error_code{EBADF, std::system_category()};
        }
        Callbacks_[io.Fd()] = std::move(callback);
        std::error_code err = AddFdToEpoll(Fd(), io.Fd(), mode);
        if (err) {
            Wg_.Dec();
        }
        return err;
    }

} // namespace NAsync
