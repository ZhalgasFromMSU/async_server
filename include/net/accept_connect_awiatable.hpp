#pragma once

#include <io/io_awaitable_base.hpp>
#include <net/socket.hpp>

#include <coroutine>
#include <optional>

namespace NAsync {

    class TAcceptAwaitable: public TWithEpoll {
    public:
        inline explicit TAcceptAwaitable(const TSocket& socket) noexcept
            : Socket_{socket}
        {}

        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle) noexcept;
        TResult<TSocket> await_resume() noexcept;

    private:
        const TSocket& Socket_;
        std::optional<TResult<TSocket>> NewSocket_;
    };

    class TConnectAwaitable: public TWithEpoll {
    public:
        inline TConnectAwaitable(const TSocket& socket, const TSocket::TAddr& remoteSock) noexcept
            : Socket_{socket}
            , RemoteSock_{remoteSock}
        {}

        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle) noexcept;
        std::error_code await_resume() noexcept;

    private:
        const TSocket& Socket_;
        const TSocket::TAddr& RemoteSock_;

        std::optional<std::error_code> ConnectError_;
    };

} // namespace NAsync