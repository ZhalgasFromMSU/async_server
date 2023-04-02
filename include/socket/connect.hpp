#pragma once

#include <socket/socket.hpp>
#include <io/io_awaitable_base.hpp>

#include <optional>
#include <coroutine>

namespace NAsync {

    class TConnectAwaitable: public TWithEpoll {
    public:
        inline TConnectAwaitable(const TSocket& socket, const TSockDescr& remoteSock) noexcept
            : Socket_{socket}
            , RemoteSock_{remoteSock}
        {}

        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle) noexcept;
        std::error_code await_resume() noexcept;

    private:
        const TSocket& Socket_;
        const TSockDescr& RemoteSock_;

        std::optional<std::error_code> ConnectError_;
    };

} // namespace NAsync
