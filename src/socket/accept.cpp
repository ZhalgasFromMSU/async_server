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
            Epoll->WatchForRead()
        }
    }

} // namespace NAsync
