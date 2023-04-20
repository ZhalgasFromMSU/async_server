#pragma once

#include <io/io_object.hpp>
#include <util/result.hpp>

namespace NAsync {

    class TPipe {
    public:
        static TResult<TPipe> Create() noexcept;

        inline const TIoObject& ReadEnd() const noexcept {
            return ReadEnd_;
        }

        inline const TIoObject& WriteEnd() const noexcept {
            return WriteEnd_;
        }

    private:
        TPipe(int readFd, int writeFd) noexcept
            : WriteEnd_{writeFd}
            , ReadEnd_{readFd}
        {}

        TIoObject WriteEnd_;
        TIoObject ReadEnd_;
    };

    // Eventfd is not readable by default
    class TEventFd: public TIoObject {
    public:
        static TResult<TEventFd> Create() noexcept;

        inline bool IsSet() const noexcept {
            return IsSet_;
        }

        void Set() noexcept; // Make it readable (poll will return that fd is ready for reading)
        void Reset() noexcept; // Nullify event fd to make it pollable

    private:
        TEventFd(int fd) noexcept
            : TIoObject{fd}
        {}

        bool IsSet_ = false;
    };

} // namespace NAsync
