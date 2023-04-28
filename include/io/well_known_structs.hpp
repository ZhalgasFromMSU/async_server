#pragma once

#include <io/io_object.hpp>
#include <util/result.hpp>

#include <atomic>

namespace NAsync {

    class TPipe {
    public:
        TPipe() noexcept;

        inline const TIoObject& ReadEnd() const noexcept {
            return ReadEnd_;
        }

        inline const TIoObject& WriteEnd() const noexcept {
            return WriteEnd_;
        }

    private:
        TIoObject WriteEnd_;
        TIoObject ReadEnd_;
    };

    // Eventfd is not readable by default
    class TEventFd: public TIoObject {
    public:
        TEventFd() noexcept;

        inline bool IsSet() const noexcept {
            return IsSet_.test();
        }

        void Set() noexcept; // Make it readable (poll will return that fd is ready for reading)

    private:
        std::atomic_flag IsSet_;
    };

} // namespace NAsync
