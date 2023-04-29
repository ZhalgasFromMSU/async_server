#include <asm-generic/errno-base.h>
#include <polling/epoll.hpp>

#include <mutex>

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
            std::error_code err;
            if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
                if (errno != EEXIST) {
                    err = std::error_code{errno, std::system_category()};
                } else if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) == -1) {
                    err = std::error_code{errno, std::system_category()};
                }
            }
            return err;
        }
    } // namespace

    // TEpoll
    TEpoll::TEpoll() noexcept
        : TIoObject{epoll_create1(0)}
    {
        VERIFY_EC(Watch(TEpoll::EMode::kRead, EventFd_, []{}));
    }

    TEpoll::~TEpoll() {
        if (!EventFd_.IsSet()) {
            Finish();
        }
    }

    void TEpoll::Start() noexcept {
        Worker_ = std::jthread {[this] {
            static constexpr size_t bSize = 1024;
            epoll_event buffer[bSize];
            while (!EventFd_.IsSet()) {
                int numReady = epoll_wait(Fd(), buffer, bSize, -1 /* timeout */);
                VERIFY_SYSCALL(numReady != -1);

                std::scoped_lock lock{Mut_};
                for (int i = 0; i < numReady; ++i) {
                    auto node = Cbs_.extract(buffer[i].data.fd);
                    VERIFY(!node.empty());
                    node.mapped().Execute();
                }
            }

            while (!Cbs_.empty()) {
                auto node = Cbs_.extract(Cbs_.begin());
                node.mapped().Execute();
            }
            Stopped_.test_and_set();
            Stopped_.notify_all();
        }};
    }

    void TEpoll::Finish() noexcept {
        EventFd_.Set();
        Stopped_.wait(false);
    }

    std::error_code TEpoll::Watch(EMode mode, const TIoObject& io, TJob callback) noexcept {
        if (EventFd_.IsSet()) {
            return std::error_code{EBADF, std::system_category()};
        }

        std::scoped_lock lock{Mut_};
        std::error_code err = AddFdToEpoll(Fd(), io.Fd(), mode);
        if (err) {
            return err;
        }
        Cbs_.emplace(io.Fd(), std::move(callback));
        return err;
    }

} // namespace NAsync
