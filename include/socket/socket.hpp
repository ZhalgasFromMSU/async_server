#pragma once

#include "resolve.hpp"
#include <io/io_object.hpp>

namespace NAsync {

    class TConnectAwaitable;
    class TAcceptAwaitable;

    class TSocket: public TIoObject {
    public:
        explicit TSocket(TSockDescr descr) noexcept;
        ~TSocket();

        inline const TSockDescr& Description() const noexcept {
            return Descr_;
        }

        std::error_code Bind() noexcept;
        std::error_code Listen() noexcept;
        TAcceptAwaitable Accept() noexcept;

        TConnectAwaitable Connect() noexcept;

    private:
        TSockDescr Descr_;
        int Fd_;
    };

} // namespace NAsync