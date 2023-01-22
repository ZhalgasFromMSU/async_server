#pragma once

#include <io/io_object.hpp>

namespace NAsync {

    class TPipeInput: public TIoObject {
    public:
        using TIoObject::Write;

    protected:
        TResult<int> Read(void* buf, int numBytesToRead, int flags = 0) const noexcept = delete;
    };

    class TPipeOutput: public TIoObject {
    public:
        using TIoObject::Read;

    protected:
        TResult<int> Write(const void* buf, int numBytesToWrite, int flags = 0) const noexcept = delete;
    };


    std::pair<TPipeOutput, TPipeInput> CreatePipe() noexcept;
    // First is writeFd, second is readFd. This way readFd will be closed first
    std::pair<TPipeInput, TPipeOutput> CreateReversedPipe() noexcept;

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