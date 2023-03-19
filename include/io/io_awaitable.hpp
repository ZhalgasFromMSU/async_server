#pragma once

#include <io/io_object.hpp>
#include <polling/epoll.hpp>
#include <thread/pool.hpp>
#include <util/result.hpp>

#include <coroutine>
#include <optional>

namespace NAsync {

    struct TWithEpoll {
        TEpoll* Epoll = nullptr;
        TThreadPool* ThreadPool = nullptr; // coroutine_handle is resumed on thread pool if specified or on epoll
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
        void await_suspend(std::coroutine_handle<> handle) const noexcept;
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
        void await_suspend(std::coroutine_handle<> handle) const noexcept;
        TResult<int> await_resume() noexcept;

    private:
        const TIoObject& IoObject_;
        const void* Buf_;
        int Num_;
        int Flags_;

        std::optional<TResult<int>> Result_;
    };

}