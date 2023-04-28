#pragma once

#include <io/io_awaitable_base.hpp>
#include <net/socket.hpp>

#include <coroutine>
#include <optional>

namespace NAsync {

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
