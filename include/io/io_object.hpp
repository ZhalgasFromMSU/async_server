#pragma once

#include "io_awaitable_base.hpp"
#include <util/result.hpp>

#include <coroutine>
#include <optional>

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

    class TReadAwaitable: public TWithEpoll {
    public:
        inline TReadAwaitable(const TIoObject& ioObject, void* buf, int num, int flags = 0) noexcept
            : IoObject_{ioObject}
            , Buf_{buf}
            , Num_{num}
            , Flags_{flags}
        {}

        bool await_ready() noexcept;
        bool await_suspend(std::coroutine_handle<> handle) const noexcept;
        TResult<int> await_resume() noexcept;

    private:
        const TIoObject& IoObject_;
        void* Buf_;
        int Num_;
        int Flags_;

        std::optional<TResult<int>> Result_;
    };

    class TWriteAwaitable: public TWithEpoll {
    public:
        inline TWriteAwaitable(const TIoObject& ioObject, const void* buf, int num, int flags = 0) noexcept
            : IoObject_{ioObject}
            , Buf_{buf}
            , Num_{num}
            , Flags_{flags}
        {}

        bool await_ready() noexcept;
        bool await_suspend(std::coroutine_handle<> handle) const noexcept;
        TResult<int> await_resume() noexcept;

    private:
        const TIoObject& IoObject_;
        const void* Buf_;
        int Num_;
        int Flags_;

        std::optional<TResult<int>> Result_;
    };

} // namespace NAsync
