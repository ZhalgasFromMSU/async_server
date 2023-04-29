#pragma once

#include "address/ip.hpp"
#include <io/io_object.hpp>

namespace NAsync {

    static constexpr uint16_t kAnyPort = 0;

    // Socket address
    using TIPv4SocketAddress = std::pair<TIPv4Address, uint16_t>;  // <host, port>
    using TIPv6SocketAddress = std::pair<TIPv6Address, uint16_t>;  // <host, port>

    using TSocketAddress = std::variant<TIPv4SocketAddress,
                                        TIPv6SocketAddress>;

    class TAcceptAwaitable;
    class TConnectAwaitable;

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

    class TAcceptAwaitable: public TWithEpoll {
    public:
        explicit TAcceptAwaitable(const TListeningSocket& acceptor) noexcept
            : Acceptor_{acceptor}
        {}

        bool await_ready() noexcept;
        bool await_suspend(std::coroutine_handle<> handle) noexcept;
        TResult<TSocket> await_resume() noexcept;

    private:
        const TListeningSocket& Acceptor_;
        std::optional<TResult<TSocket>> Socket_;
    };

    class TConnectAwaitable: public TWithEpoll {
    public:
        explicit TConnectAwaitable(const TSocket& socket) noexcept
            : Socket_{socket}
        {}

        bool await_ready() noexcept;
        bool await_suspend(std::coroutine_handle<> handle) noexcept;
        std::error_code await_resume() noexcept;

    private:
        const TSocket& Socket_;

        std::optional<std::error_code> Error_;
    };

} // namespace NAsync
