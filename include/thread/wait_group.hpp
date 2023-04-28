#pragma once

#include <atomic>
#include <limits>

namespace NAsync {

    // Like golang's sync.WaitGroup
    // Despite std::latch and std::barrier, allow for counter increase
    class TWaitGroup {
    public:
        TWaitGroup(int init = 0) noexcept
            : Counter_{init}
        {}

        bool Inc() noexcept;
        void Dec() noexcept;
        void Block() noexcept;
        void Wait() noexcept;

        int Count() const noexcept;

        inline bool Waited() const noexcept {
            return Counter_ == NegZero_;
        }

    private:
        static constexpr int NegZero_ = std::numeric_limits<int>::min();

        std::atomic<int> Counter_;
    };

} // namespace NAsync
