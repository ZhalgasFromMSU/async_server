#include <socket/socket.hpp>
#include <socket/accept.hpp>
#include <socket/connect.hpp>
#include <util/result.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>

namespace NAsync {

    TSocket::TSocket(int fd, TSockDescr descr) noexcept
        : TIoObject{fd}
        , Descr_{std::move(descr)}
    {}

    std::error_code TSocket::Bind() noexcept {
        sockaddr* addr = static_cast<sockaddr*>((void*)Descr_.AddrInfo_.get());
        socklen_t size;
        switch (Descr_.Domain()) {
            case EDomain::kIPv4:
                size = sizeof(sockaddr_in);
                break;
            case EDomain::kIPv6:
                size = sizeof(sockaddr_in6);
                break;
            case EDomain::kUnix:
                size = sizeof(sockaddr_un);
                break;
            default:
                VERIFY(false);
        }

        int status = bind(Fd(), addr, size);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return std::error_code{};
    }

    std::error_code TSocket::Listen() noexcept {
        int status = listen(Fd(), 10);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return std::error_code{};
    }

    TAcceptAwaitable TSocket::Accept() noexcept {
        return TAcceptAwaitable{*this};
    }

    TConnectAwaitable TSocket::Connect(const TSockDescr& remoteSock) noexcept {
        return TConnectAwaitable{*this, remoteSock};
    }

} // namespace NAsync
