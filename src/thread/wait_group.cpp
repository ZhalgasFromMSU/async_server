#include <thread/wait_group.hpp>
#include <util/result.hpp>

#include <future>

namespace NAsync {

    bool TWaitGroup::Inc() noexcept {
        int current = Counter_;
        while (current >= 0) {
            if (Counter_.compare_exchange_weak(current, current + 1)) {
                return true;
            }
        }
        return false;
    }

    void TWaitGroup::Dec() noexcept {
        int current = Counter_;
        while (true) {
            VERIFY(current != 0 && current != NegZero_);
            int newVal;
            if (current > 0) {
                newVal = current - 1;
            } else if (current == -1) {
                newVal = NegZero_;
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

    void TWaitGroup::Block() noexcept {
        if (Counter_ < 0) {
            return;
        }

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
    }

    void TWaitGroup::Wait() noexcept {
        int current = Counter_;
        while (Counter_ != NegZero_) {
            Counter_.wait(current);
            current = Counter_;
        }
    }

    int TWaitGroup::Count() const noexcept {
        int count = Counter_;
        if (count >= 0) {
            return count;
        } else if (count == NegZero_) {
            return 0;
        } else {
            return -count;
        }
    }

} // namespace NAsync
