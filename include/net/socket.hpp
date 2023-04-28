#pragma once

#include "address/ip.hpp"
#include <io/io_object.hpp>

#include <variant>
#include <optional>

namespace NAsync {

    static constexpr uint16_t kAnyPort = 0;

    // Socket address
    using TIPv4SocketAddress = std::pair<TIPv4Address, uint16_t>;  // <host, port>
    using TIPv6SocketAddress = std::pair<TIPv6Address, uint16_t>;  // <host, port>

    using TSocketAddress = std::variant<TIPv4SocketAddress,
                                        TIPv6SocketAddress>;

    // Forward declarations
    class TAcceptAwaitable;
    class TConnectAwaitable;

    // TListeningSocket
    class TListeningSocket : public TIoObject {
    public:
        static TResult<TListeningSocket> Create(TSocketAddress address, int queueLen = 10) noexcept;

        inline const TSocketAddress& Address() const noexcept {
            return Address_;
        }

        TAcceptAwaitable Accept() const noexcept;

    private:
        TListeningSocket(int sockFd, TSocketAddress address) noexcept
            : TIoObject{sockFd}
            , Address_{std::move(address)}
        {}

        TSocketAddress Address_;
    };

    // TSocket
    class TSocket : public TIoObject {
    public:
        template<std::derived_from<IAddress> T>
        static TResult<TSocket> Create(bool streamSocket) noexcept;

        TConnectAwaitable Connect(TSocketAddress dest) noexcept;

        inline const std::optional<TSocketAddress>& RemoteAddress() const noexcept {
            return Remote_;
        }

    private:
        friend TAcceptAwaitable;

        TSocket(int sockFd) noexcept
            : TIoObject{sockFd}
        {}

        TSocket(int sockFd, TSocketAddress remote) noexcept
            : TIoObject{sockFd}
            , Remote_{std::move(remote)}
        {}

        std::optional<TSocketAddress> Remote_; // udp sockets may lack remote address
    };

} // namespace NAsync
