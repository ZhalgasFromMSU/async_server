module;

#include <atomic>
#include <limits>

export module async:waitgroup;

namespace async {

  export class WaitGroup {
  public:
    WaitGroup() noexcept = default;

    explicit WaitGroup(int init) noexcept
        : counter_{init} {
    }

    bool TryInc() noexcept {
      int current = counter_.load(std::memory_order_relaxed);
      while (current >= 0) {
        if (counter_.compare_exchange_weak(current, current + 1,
                                           std::memory_order_relaxed)) {
          if (current == 0) {
            counter_.notify_all();
          }
          return true;
        }
      }
      return false;
    }

    bool TryDec() noexcept {
      int current = counter_.load(std::memory_order_relaxed);
      while (current != 0 && current != neg_zero_) {
        int next;
        if (current == -1) {
          next = neg_zero_;
        } else if (current < 0) {
          next = current + 1;
        } else {
          next = current - 1;
        }

        if (counter_.compare_exchange_strong(current, next,
                                             std::memory_order_relaxed)) {
          if (next == neg_zero_) {
            counter_.notify_all();
          }
          return true;
        }
      }
      return false;
    }

    // Wait for decrease to be possible and decrease counter
    bool Dec() noexcept {
      int current = counter_.load(std::memory_order_relaxed);
      while (current != neg_zero_) {
        if (current == 0) {
          counter_.wait(0, std::memory_order_relaxed);
          current = counter_.load(std::memory_order_relaxed);
        } else {
          int next;
          if (current == -1) {
            next = neg_zero_;
          } else if (current < 0) {
            next = current + 1;
          } else {
            next = current - 1;
          }

          if (counter_.compare_exchange_strong(current, next,
                                               std::memory_order_relaxed)) {
            if (next == neg_zero_) {
              counter_.notify_all();
            }
            return true;
          }
        }
      }
      return false;
    }

    void Block() noexcept {
      int current = counter_.load(std::memory_order_relaxed);
      while (current >= 0) {
        int next;
        if (current == 0) {
          next = neg_zero_;
        } else {
          next = -current;
        }

        if (counter_.compare_exchange_strong(current, next,
                                             std::memory_order_relaxed)) {
          if (next == neg_zero_) {
            counter_.notify_all();
          }
        }
      }
    }

    // Wait until counter is in terminal state
    void Wait() noexcept {
      int current = counter_.load(std::memory_order_relaxed);
      while (current != neg_zero_) {
        counter_.wait(current, std::memory_order_relaxed);
        current = counter_.load(std::memory_order_relaxed);
      }
    }

  private:
    static constexpr int neg_zero_ = std::numeric_limits<int>::min();
    std::atomic<int> counter_ = 0;
  };

} // namespace async
