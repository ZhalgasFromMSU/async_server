#pragma once

#include <condition_variable>
#include <system_error>
#include <type_traits>

namespace NAsync {

    // Same as golang's sync.WaitGroup, but Wait has timeout
    class TWaitGroup {
    public:
        void Add(int delta = 1) noexcept;
        void Done() noexcept;
        void Block() noexcept;

        void Wait() noexcept;

        // return true if waited successfuly (Counter_ == 0)
        bool WaitFor(std::chrono::microseconds timeout) noexcept;

    private:
        // Can't use std::atomic here, because it doesn't have wait_for method
        bool Blocked_ = false;
        int Counter_ = 0;
        std::mutex Mutex_;
        std::condition_variable CondVar_;
    };

} // namespace NAsync
