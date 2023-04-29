#include <io/io_awaitable_base.hpp>

namespace NAsync {

    bool TWithEpoll::Suspend(TEpoll::EMode mode, const TIoObject &io, std::coroutine_handle<> handle) const {
        return Runtime_->Schedule(mode, io, handle);
    }

} // namespace NAsync
