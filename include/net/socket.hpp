#pragma once

#include "address/ip.hpp"
#include <io/io_object.hpp>

#include <variant>

namespace NAsync {

    class TAcceptAwaitable;
    class TConnectAwaitable;

    class TSocket: public TIoObject {
    public:
        enum class EType {
            kUdp,
            kTcp,
        };

        using TAddr = std::variant<
            std::pair<TIPv4Address, uint16_t>, // <host, port>
            std::pair<TIPv6Address, uint16_t>  // <host, port>
        >;

        static TResult<TSocket> CreateListeningSocket(EType type, TAddr addr) noexcept;

        TSocket(int fd, EType type, TAddr addr) noexcept;

        inline EType Type() const noexcept {
            return Type_;
        }

        inline const TAddr& Addr() const noexcept {
            return Address_;
        }

        TAcceptAwaitable Accept() const noexcept;
        TConnectAwaitable Connect(const TAddr& addr) const noexcept;

    private:
        static constexpr int ListenintQueueSize_ = 10;

        EType Type_;
        TAddr Address_;
    };

} // namespace NAsync