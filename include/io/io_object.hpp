#pragma once

#include <util/result.hpp>

namespace NAsync {

    class TIoObject {
    public:
        explicit TIoObject(int fd = -1) noexcept;
        ~TIoObject() noexcept;

        TIoObject& operator=(const TIoObject&) = delete;
        TIoObject(const TIoObject&) = delete;

        TIoObject& operator=(TIoObject&& other) noexcept;
        TIoObject(TIoObject&& other) noexcept;

        int Fd() const noexcept;

    protected:
        int Fd_;
    };

    TResult<int> Read(const TIoObject& ioObject, void* buf, int num, int flags = 0) noexcept;
    TResult<int> Write(const TIoObject& ioObject, const void* buf, int num, int flags = 0) noexcept;

} // namespace NAsync