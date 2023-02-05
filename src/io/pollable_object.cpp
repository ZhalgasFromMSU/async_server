#include <io/pollable_object.hpp>

namespace NAsync {

    TReadPollable::TReadPollable(const TIoObject& io, void* buf, int num, int flags) noexcept
        : Io_{io}
        , Buf_{buf}
        , Num_{num}
        , Flags_{flags}
    {}

    std::error_code TReadPollable::ScheduleToEpoll(TEpoll& epoll, TEpoll::TCallback callback) const noexcept {
        return epoll.WatchForRead(Io_, std::move(callback));
    }

    std::optional<TResult<int>> TReadPollable::Try() const noexcept {
        TResult<int> result = Read(Io_, Buf_, Num_, Flags_);
        if (!result && (result.Error().value() == EWOULDBLOCK || result.Error().value() == EAGAIN)) {
            return std::nullopt;
        }
        return result;
    }

    TWritePollable::TWritePollable(const TIoObject& io, const void* buf, int num, int flags) noexcept
        : Io_{io}
        , Buf_{buf}
        , Num_{num}
        , Flags_{flags}
    {}

    std::error_code TWritePollable::ScheduleToEpoll(TEpoll& epoll, TEpoll::TCallback callback) const noexcept {
        return epoll.WatchForWrite(Io_, std::move(callback));
    }

    std::optional<TResult<int>> TWritePollable::Try() const noexcept {
        TResult<int> result = Write(Io_, Buf_, Num_, Flags_);
        if (!result && (result.Error().value() == EWOULDBLOCK || result.Error().value() == EAGAIN)) {
            return std::nullopt;
        }
        return result;
    }

} // namespace NAsync