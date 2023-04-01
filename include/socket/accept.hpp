#pragma once

#include <socket/socket.hpp>
#include <io/io_awaitable_base.hpp>

namespace NAsync {

    class TAcceptAwaitable: public TWithEpoll {
    public:
        TAcceptAwaitable(const TSocket& socket) noexcept;

    private:
        const TSocket& Socket_;
    };

} // namespace NAsync
