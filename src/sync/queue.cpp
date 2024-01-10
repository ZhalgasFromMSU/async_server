module;

#include <algorithm>
#include <atomic>
#include <optional>
#include <vector>

export module async:queue;

namespace async {

  export template<typename T>
    requires(std::is_nothrow_move_constructible_v<T>)
  class Queue {
  public:
    Queue(std::size_t max_size_hint, std::size_t num_writers,
          std::size_t num_readers) noexcept
        : num_writers_{num_writers}
        , num_readers_{num_readers}
        , data_(std::max({max_size_hint, num_writers, num_readers})) {
    }

    ~Queue() {
      for (std::size_t i = read_idx_.load(); i < write_idx_.load(); ++i) {
        data_[i % data_.size()].payload.obj.~T();
      }
    }

    template<typename... Args>
    bool TryPush(Args&&... obj) noexcept {
      std::size_t idx;
      while (true) {
        std::size_t cur_w_idx = write_idx_.load(std::memory_order_relaxed);
        std::size_t cur_r_idx = read_idx_.load(std::memory_order_relaxed);
        if (cur_w_idx >= cur_r_idx && cur_w_idx - cur_r_idx >= data_.size()) {
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

      SetPayload(data_[idx % data_.size()], std::forward<Args>(obj)...);
      return true;
    }

    std::optional<T> TryPop() noexcept {
      std::size_t idx;
      while (true) {
        std::size_t cur_w_idx = write_idx_.load(std::memory_order_relaxed);
        std::size_t cur_r_idx = read_idx_.load(std::memory_order_relaxed);
        if (cur_w_idx <= cur_r_idx) {
          return std::nullopt;
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

      return RetrievePayload(data_[idx % data_.size()]);
    }

    template<typename... Args>
    void Push(Args&&... obj) noexcept {
      SetPayload(data_[write_idx_.fetch_add(1, std::memory_order_relaxed) %
                       data_.size()],
                 std::forward<Args>(obj)...);
    }

    T Pop() noexcept {
      return RetrievePayload(
          data_[read_idx_.fetch_add(1, std::memory_order_relaxed) %
                data_.size()]);
    }

  private:
    union Payload {
      T obj;

      Payload() noexcept {
      }

      ~Payload() {
      }
    };

    struct Node {
      Payload payload;
      std::atomic_flag available;
    };

    template<typename... Args>
    static void SetPayload(Node& node, Args&&... obj) {
      node.available.wait(true, std::memory_order_acquire);
      new (&node.payload.obj) T(std::forward<Args>(obj)...);
      node.available.test_and_set(std::memory_order_release);
      node.available.notify_one();
    }

    static T RetrievePayload(Node& node) {
      node.available.wait(false, std::memory_order_acquire);
      T ret = std::move(node.payload.obj);
      node.payload.obj.~T();
      node.available.clear(std::memory_order_release);
      node.available.notify_one();
      return ret;
    }

    std::size_t num_writers_;
    std::size_t num_readers_;
    std::vector<Node> data_;
    std::atomic<std::size_t> write_idx_ = 0;
    std::atomic<std::size_t> read_idx_ = 0;
  };

} // namespace async
