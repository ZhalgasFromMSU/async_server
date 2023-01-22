#pragma once

#include <util/result.hpp>

namespace NAsync {

    class TIoObject {
    public:
        TIoObject(int fd) noexcept;
        ~TIoObject() noexcept;

        TIoObject& operator=(const TIoObject&) = delete;
        TIoObject(const TIoObject&) = delete;

        TIoObject& operator=(TIoObject&& other) noexcept;
        TIoObject(TIoObject&& other) noexcept;

        int Fd() const noexcept;

    protected:
        TResult<int> Write(const void* buf, int numBytesToWrite, int flags = 0) const noexcept;
        TResult<int> Read(void* buf, int numBytesToRead, int flags = 0) const noexcept;

    private:
        int Fd_ = -1;
    };

} // namespace NAsync