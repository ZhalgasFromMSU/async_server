#pragma once

#include "socket_descr.hpp"
#include <io/io_object.hpp>

namespace NAsync {

    class TConnectAwaitable;
    class TAcceptAwaitable;

    class TSocket: public TIoObject {
    public:
        TSocket(int fd, TSockDescr descr) noexcept;

        inline const TSockDescr& Description() const noexcept {
            return Descr_;
        }

        std::error_code Bind() noexcept;
        std::error_code Listen() noexcept;
        TAcceptAwaitable Accept() noexcept;

        TConnectAwaitable Connect() noexcept;

    private:
        TSockDescr Descr_;
    };

} // namespace NAsync