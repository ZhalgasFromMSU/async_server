#include "net/socket.hpp"
#include <net/accept_connect_awiatable.hpp>

#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <utility>
#include <variant>

namespace NAsync {

    namespace {
        TSocketAddress ToSocketAddress(const sockaddr& raw) {
            if (raw.sa_family == AF_INET) {
                const auto* rawV4 = static_cast<const sockaddr_in*>(static_cast<const void*>(&raw));
                return TSocketAddress{
                    std::in_place_type<TIPv4SocketAddress>,
                    std::piecewise_construct,
                    std::forward_as_tuple(rawV4->sin_addr),
                    std::forward_as_tuple(rawV4->sin_port)
                };
            } else if (raw.sa_family == AF_INET6) {
                const auto* rawV6 = static_cast<const sockaddr_in6*>(static_cast<const void*>(&raw));
                return TSocketAddress{
                    std::in_place_type<TIPv6SocketAddress>,
                    std::piecewise_construct,
                    std::forward_as_tuple(rawV6->sin6_addr),
                    std::forward_as_tuple(rawV6->sin6_port)
                };
            } else {
                VERIFY(false);
            }
        }

    }

    // Accept

    // Connect

    TResult<TSocket> TConnectAwaitable::await_resume() noexcept {
        //if (ConnectError_) {
            //return std::move(*ConnectError_);
        //}
        //int status = Connect(Socket_.Fd(), RemoteAddr_);
        //if (status == -1) {
            //return std::error_code{errno, std::system_category()};
        //}
        return std::error_code{};
    }

} // namespace NAsync
