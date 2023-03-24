#pragma once

#include <socket/address.hpp>
#include <io/io_awaitable_base.hpp>

#include <coroutine>
#include <optional>

namespace NAsync {

    class TConnectAwaitable;
    class TAcceptAwaitable;

    class TSocket {
    public:
        enum class EDomain {
            kLocal,
            kIpV4,
            kIpV6,
        };

        enum class ESocketType {
            kTcp,
            kUdp,
        };

        TSocket(EDomain domain, ESocketType type) noexcept;

    };

    class TConnectAwaitable: public TWithEpoll {
    public:
        TConnectAwaitable(const TSocket& socket, const TAddress& address);

        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle) noexcept;
        std::error_code await_resume() noexcept; // on success error_code is empty

    private:
        const TSocket& Socket_;
        const TAddress& Address_;
        std::optional<std::error_code> ConnectionError_;
    };

} // namespace NAsync