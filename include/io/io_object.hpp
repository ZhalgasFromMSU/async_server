#pragma once

namespace NAsync {

    class TReadAwaitable;
    class TWriteAwaitable;

    class TIoObject {
    public:
        // Constructors
        explicit TIoObject(int fd = -1) noexcept;
        TIoObject& operator=(const TIoObject&) = delete;
        TIoObject(const TIoObject&) = delete;
        TIoObject& operator=(TIoObject&& other) noexcept;
        TIoObject(TIoObject&& other) noexcept;
        ~TIoObject() noexcept;

        // Accessors
        int Fd() const noexcept;

        // Methods
        TReadAwaitable Read(void* buf, int num, int flags = 0) const noexcept;
        TWriteAwaitable Write(const void* buf, int num, int flags = 0) const noexcept;

    protected:
        int Fd_;
    };

} // namespace NAsync