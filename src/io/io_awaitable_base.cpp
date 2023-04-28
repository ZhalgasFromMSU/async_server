#include <io/io_awaitable_base.hpp>

namespace NAsync {

    bool TWithEpoll::Suspend(TEpoll::EMode mode, const TIoObject &io, std::coroutine_handle<> handle) const {
        if (!ThreadPool_) {
            return !Epoll_->Watch(mode, io, handle);
        } else {
            return !Epoll_->Watch(mode, io, [this, handle] {
                VERIFY(ThreadPool_->EnqueJob(handle));
            });
        }
    }

} // namespace NAsync
