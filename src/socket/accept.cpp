#include <socket/accept.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace NAsync {

    bool TAcceptAwaitable::await_ready() noexcept {
        sockaddr addr;
        socklen_t size;
        int sockFd = accept4(Socket_.Fd(), &addr, &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            if (errno & (EAGAIN || EWOULDBLOCK)) {
                return false;
            }
            NewSocket_ = std::error_code{errno, std::system_category()};
        }
        NewSocket_ = TSocket{sockFd, TSockDescr{Socket_.Description().Domain(), Socket_.Description().Type(), addr}};
        return true;
    }

    void TAcceptAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        if (ThreadPool) {
            Epoll->WatchForRead(Socket_, [this, handle] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->WatchForRead(Socket_, handle);
        }
    }

    TResult<TSocket> TAcceptAwaitable::await_resume() noexcept {
        if (NewSocket_) {
            return std::move(*NewSocket_);
        }

        sockaddr addr;
        socklen_t size;
        int sockFd = accept4(Socket_.Fd(), &addr, &size, SOCK_NONBLOCK);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd, TSockDescr{Socket_.Description().Domain(), Socket_.Description().Type(), addr}};
    }

} // namespace NAsync
