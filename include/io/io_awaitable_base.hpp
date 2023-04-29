#pragma once

#include <coro/runtime.hpp>

namespace NAsync {

    class TWithEpoll {
    public:
        inline bool HasRuntime() {
            return Runtime_ != nullptr;
        }

        inline void SetRuntime(TRuntime* runtime) {
            Runtime_ = runtime;
        }

        bool Suspend(TEpoll::EMode mode, const TIoObject& io, std::coroutine_handle<> handle) const;

    private:
        TRuntime* Runtime_ = nullptr;
    };

} // namespace NAsync
