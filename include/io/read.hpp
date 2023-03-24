#pragma once

#include <io/io_awaitable_base.hpp>
#include <util/result.hpp>

#include <optional>
#include <coroutine>

namespace NAsync {

    TResult<int> Read(const TIoObject& ioObject, void* buf, int num, int flags = 0) noexcept;

    class TReadAwaitable: public TWithEpoll {
    public:
        inline TReadAwaitable(const TIoObject& ioObject, void* buf, int num, int flags = 0) noexcept
            : IoObject_{ioObject}
            , Buf_{buf}
            , Num_{num}
            , Flags_{flags}
        {}

        bool await_ready() noexcept;
        void await_suspend(std::coroutine_handle<> handle) const noexcept;
        TResult<int> await_resume() noexcept;

    private:
        const TIoObject& IoObject_;
        void* Buf_;
        int Num_;
        int Flags_;

        std::optional<TResult<int>> Result_;
    };

} // namespace NAsync