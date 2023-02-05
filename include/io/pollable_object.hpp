#pragma once

#include <io/io_object.hpp>
#include <polling/epoll.hpp>

#include <optional>

namespace NAsync {

    template<typename T>
    concept CPollable = requires(
        const T t,
        TEpoll& epoll,
        TEpoll::TCallback callback
    ) {
        { t.Try() } noexcept -> std::same_as<std::optional<TResult<int>>>;
        { t.ScheduleToEpoll(epoll, callback) } -> std::same_as<std::error_code>; // TODO make this method noexcept also
    };

    class TReadPollable {
    public:
        TReadPollable(const TIoObject& io, void* buf, int num, int flags = 0) noexcept;

        std::error_code ScheduleToEpoll(TEpoll& epoll, TEpoll::TCallback callback) const noexcept;
        std::optional<TResult<int>> Try() const noexcept;

    private:
        const TIoObject& Io_;
        void* Buf_;
        int Num_;
        int Flags_;
    };

    class TWritePollable {
    public:
        TWritePollable(const TIoObject& io, const void* buf, int num, int flags = 0) noexcept;

        std::error_code ScheduleToEpoll(TEpoll& epoll, TEpoll::TCallback callback) const noexcept;
        std::optional<TResult<int>> Try() const noexcept;

    private:
        const TIoObject& Io_;
        const void* Buf_;
        int Num_;
        int Flags_;
    };

} // namespace NAsync