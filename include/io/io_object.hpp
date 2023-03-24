#pragma once

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

} // namespace NAsync