#pragma once

#include <io/io_object.hpp>

namespace NAsync {

    class TPipe {
    public:
        static TPipe Create() noexcept;

        const TIoObject& ReadEnd() const noexcept;
        const TIoObject& WriteEnd() const noexcept;

    private:
        TPipe(int readFd, int writeFd) noexcept;

        TIoObject WriteEnd_;
        TIoObject ReadEnd_;
    };

    // Eventfd is not readable by default
    class TEventFd: public TIoObject {
    public:
        TEventFd() noexcept;

        bool IsSet() const noexcept;
        void Set() noexcept; // Make it readable (poll will return that fd is ready for reading)
        void Reset() noexcept; // Nullify event fd to make it pollable

    private:
        bool IsSet_ = false;
    };

} // namespace NAsync