#pragma once

#include <socket/socket.hpp>
#include <io/io_awaitable_base.hpp>

namespace NAsync {

    class TConnectAwaitable: public TWithEpoll {
    public:
        inline explicit TConnectAwaitable(const TSocket& socket) noexcept
            : Socket_{socket}
        {}

    private:
        const TSocket& Socket_;
    };

} // namespace NAsync
