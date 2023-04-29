#include <coro/runtime.hpp>

namespace NAsync {

    TRuntime::TRuntime(size_t threads) noexcept {
        if (threads > 1) {
            Tp_.emplace(threads - 1);
        }
    }

    TRuntime::~TRuntime() {
        Stop();
    }

    void TRuntime::Start() noexcept {
        if (Tp_) {
            Tp_->Start();
        }
        Epoll_.Start();
    }

    void TRuntime::Stop() noexcept {
        Epoll_.Finish();
        if (Tp_) {
            Tp_->Finish();
        }
    }

    void TRuntime::Execute(std::coroutine_handle<> handle) noexcept {
        if (Tp_) {
            VERIFY(Tp_->EnqueJob(handle));
        } else {
            handle.resume();
        }
    }

    bool TRuntime::Schedule(TEpoll::EMode mode, const TIoObject &io, std::coroutine_handle<> handle) noexcept {
        if (Tp_) {
            return !Epoll_.Watch(mode, io, handle);
        } else {
            return !Epoll_.Watch(mode, io, [this, handle] {
                VERIFY(Tp_->EnqueJob(handle));
            });
        }
    }

} // namespace NAsync
