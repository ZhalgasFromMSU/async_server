#include "converter.hpp"
#include <net/accept_connect_awiatable.hpp>

namespace NAsync {

    // TAcceptAwaitable
    bool TAcceptAwaitable::await_ready() noexcept {
        NPrivate::TConverter cv;
        int sockFd = accept4(Acceptor_.Fd(), cv.MutRaw(), cv.MutSize(), SOCK_NONBLOCK);
        if (sockFd == -1) {
            if (errno & (EAGAIN | EWOULDBLOCK)) {
                return false;
            }
            Socket_.emplace(std::error_code{errno, std::system_category()});
        } else {
            Socket_.emplace(TSocket{sockFd, cv.FromRawStorage()});
        }
        return true;
    }

    void TAcceptAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        if (ThreadPool) {
            Epoll->Watch(TEpoll::EMode::kRead, Acceptor_, [this, handle] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->Watch(TEpoll::EMode::kRead, Acceptor_, handle);
        }
    }

    TResult<TSocket> TAcceptAwaitable::await_resume() noexcept {
        if (Socket_) {
            return std::move(*Socket_);
        }

        NPrivate::TConverter cv;
        int sockFd = accept4(Acceptor_.Fd(), cv.MutRaw(), cv.MutSize(), SOCK_NONBLOCK);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd, cv.FromRawStorage()};
    }

} // namespace NAsync
