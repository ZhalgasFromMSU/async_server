module;

#include <algorithm>
#include <atomic>
#include <optional>
#include <vector>

export module async:queue;
import :optional;

namespace async {

  export template<typename T>
  class Queue {
  public:
    Queue(std::size_t max_size_hint, std::size_t num_writers,
          std::size_t num_readers) noexcept
        : num_writers_{num_writers}
        , num_readers_{num_readers}
        , data_(std::max({max_size_hint, num_writers, num_readers})) {
    }

    template<typename... Args>
    bool TryPush(Args&&... obj) noexcept {
      std::size_t idx;
      while (true) {
        std::size_t cur_w_idx = write_idx_.load(std::memory_order_relaxed);
        std::size_t cur_r_idx = read_idx_.load(std::memory_order_relaxed);
        if ((cur_w_idx >= cur_r_idx && cur_w_idx - cur_r_idx >= data_.size()) ||
            !data_[cur_w_idx % data_.size()].IsEmpty()) {
          return false;
        }

        if (num_writers_ == 1) {
          write_idx_.store(cur_w_idx + 1, std::memory_order_relaxed);
          idx = cur_w_idx;
          break;
        } else if (write_idx_.compare_exchange_strong(
                       cur_w_idx, cur_w_idx + 1, std::memory_order_relaxed)) {
          idx = cur_w_idx;
          break;
        }
      }

      data_[idx % data_.size()].PushUnsafe(std::forward<Args>(obj)...);
      return true;
    }

    std::conditional_t<std::is_same_v<T, void>, bool, std::optional<T>>
    TryPop() noexcept {
      std::size_t idx;
      while (true) {
        std::size_t cur_w_idx = write_idx_.load(std::memory_order_relaxed);
        std::size_t cur_r_idx = read_idx_.load(std::memory_order_relaxed);
        if (cur_w_idx <= cur_r_idx ||
            !data_[cur_r_idx % data_.size()].IsReady()) {
          if constexpr (std::is_same_v<T, void>) {
            return false;
          } else {
            return std::nullopt;
          }
        }

        if (num_readers_ == 1) {
          read_idx_.store(cur_r_idx + 1, std::memory_order_relaxed);
          idx = cur_r_idx;
          break;
        } else if (read_idx_.compare_exchange_strong(
                       cur_r_idx, cur_r_idx + 1, std::memory_order_relaxed)) {
          idx = cur_r_idx;
          break;
        }
      }

      if constexpr (std::is_same_v<T, void>) {
        data_[idx % data_.size()].PopUnsafe();
        return true;
      } else {
        return data_[idx % data_.size()].PopUnsafe();
      }
    }

    template<typename... Args>
    void Push(Args&&... obj) noexcept {
      data_[write_idx_.fetch_add(1, std::memory_order_relaxed) % data_.size()]
          .Push(std::forward<Args>(obj)...);
    }

    T Pop() noexcept {
      return data_[read_idx_.fetch_add(1, std::memory_order_relaxed) %
                   data_.size()]
          .Pop();
    }

  private:
    std::size_t num_writers_;
    std::size_t num_readers_;
    std::vector<Optional<T>> data_;
    std::atomic<std::size_t> write_idx_ = 0;
    std::atomic<std::size_t> read_idx_ = 0;
  };

} // namespace async
