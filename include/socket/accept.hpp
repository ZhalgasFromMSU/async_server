#pragma once

#include <socket/socket.hpp>
#include <io/io_awaitable_base.hpp>

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
        std::error_code await_resume() noexcept;

    private:
        const TSocket& Socket_;
        std::optional<std::error_code> NewSocket_;
    };

} // namespace NAsync
