#pragma once

#include <socket/socket.hpp>
#include <io/io_awaitable_base.hpp>

namespace NAsync {

    class TConnectAwaitable: public TWithEpoll {
    public:
        TConnectAwaitable(const TSocket& socket) noexcept;

    private:
        const TSocket& Socket_;
    };

} // namespace NAsync
