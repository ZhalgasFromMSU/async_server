#include <socket/connect.hpp>

#include <netdb.h>
#include <sys/socket.h>

namespace NAsync {

    bool TConnectAwaitable::await_ready() noexcept {
        int status = connect(Socket_.Fd(), RemoteSock_.AddrInfo().ai_addr, RemoteSock_.AddrInfo().ai_addrlen);
        if (status == -1) {
            if (errno & (EINPROGRESS | EAGAIN)) {
                return false;
            }
            ConnectError_ = std::error_code{errno, std::system_category()};
        }
        ConnectError_ = std::error_code{};
        return true;
    }

    void TConnectAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        if (ThreadPool) {
            Epoll->WatchForWrite(Socket_, [this, handle] {
                VERIFY(ThreadPool->EnqueJob(handle));
            });
        } else {
            Epoll->WatchForWrite(Socket_, handle);
        }
    }

    std::error_code TConnectAwaitable::await_resume() noexcept {
        if (ConnectError_) {
            return std::move(*ConnectError_);
        }
        int status = connect(Socket_.Fd(), RemoteSock_.AddrInfo().ai_addr, RemoteSock_.AddrInfo().ai_addrlen);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return std::error_code{};
    }

} // namespace NAsync
