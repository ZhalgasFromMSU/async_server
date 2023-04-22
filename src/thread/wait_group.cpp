#include <thread/wait_group.hpp>
#include <util/result.hpp>

#include <mutex>

namespace NAsync {

    bool TCounter::Inc() noexcept {
        int current = Counter_;
        while (current >= 0) {
            if (Counter_.compare_exchange_weak(current, current + 1)) {
                return true;
            }
        }
        return false;
    }

    void TCounter::Dec() noexcept {
        int current = Counter_;
        while (true) {
            VERIFY(current != 0 && current != NegZero_);
            int newVal;
            if (current > 0) {
                newVal = current - 1;
            } else {  // current < 0
                newVal = current + 1;
            }
            if (Counter_.compare_exchange_weak(current, newVal)) {
                break;
            }
        }

        if (Counter_ == NegZero_) {
            Counter_.notify_all();
        }
    }

    void TCounter::BlockAndWait() noexcept {
        int current = Counter_;
        while (true) {
            int newVal;
            if (current > 0) {
                newVal = -current;
            } else if (current == 0) {
                newVal = NegZero_;
            }
            if (Counter_.compare_exchange_weak(current, newVal)) {
                break;
            }
        }

        current = Counter_;
        while (Counter_ != NegZero_) {
            Counter_.wait(current);
            current = Counter_;
        }
    }

} // namespace NAsync
