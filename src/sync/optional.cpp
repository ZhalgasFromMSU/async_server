module;

#include <atomic>
#include <optional>

export module async:optional;

namespace async {

  export template<typename T>
    requires(std::is_nothrow_move_constructible_v<T> || std::is_same_v<T, void>)
  class Optional {
  public:
    Optional() noexcept = default;

    ~Optional() {
      if (status_ == Status::kReady) {
        payload_.obj.~T();
      }
    }

    template<typename... Args>
    bool TryPush(Args&&... args) noexcept {
      Status empty = Status::kEmpty;
      if (!status_.compare_exchange_strong(empty, Status::kInProgress,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
        return false;
      }
      new (&payload_.obj) T(std::forward<Args>(args)...);
      status_.store(Status::kReady, std::memory_order_release);
      return true;
    }

    std::optional<T> TryPop() noexcept {
      Status ready = Status::kReady;
      if (!status_.compare_exchange_strong(ready, Status::kInProgress,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
        return std::nullopt;
      }
      std::optional<T> ret{std::in_place, std::move(payload_.obj)};
      payload_.obj.~T();
      status_.store(Status::kEmpty, std::memory_order_release);
      return ret;
    }

    template<typename... Args>
    void PushUnsafe(Args&&... args) noexcept {
      new (&payload_.obj) T(std::forward<Args>(args)...);
      status_.store(Status::kReady, std::memory_order_release);
    }

    T PopUnsafe() noexcept {
      T ret = std::move(payload_.obj);
      payload_.obj.~T();
      status_.store(Status::kEmpty, std::memory_order_release);
      return ret;
    }

    template<typename... Args>
    void Push(Args&&... args) noexcept {
      Status empty = Status::kEmpty;
      while (!status_.compare_exchange_strong(empty, Status::kInProgress,
                                              std::memory_order_acquire,
                                              std::memory_order_relaxed)) {
        status_.wait(empty, std::memory_order_relaxed);
        empty = Status::kEmpty;
      }
      new (&payload_.obj) T(std::forward<Args>(args)...);
      status_.store(Status::kReady, std::memory_order_release);
      status_.notify_one();
    }

    T Pop() noexcept {
      Status ready = Status::kReady;
      while (!status_.compare_exchange_strong(ready, Status::kInProgress,
                                              std::memory_order_acquire,
                                              std::memory_order_release)) {
        status_.wait(ready, std::memory_order_relaxed);
        ready = Status::kReady;
      }
      T ret = std::move(payload_.obj);
      payload_.obj.~T();
      status_.store(Status::kEmpty, std::memory_order_release);
      status_.notify_one();
      return ret;
    }

    bool IsReady() noexcept {
      return status_.load(std::memory_order_acquire) == Status::kReady;
    }

    bool IsEmpty() noexcept {
      return status_.load(std::memory_order_acquire) == Status::kEmpty;
    }

  private:
    enum class Status {
      kEmpty,
      kInProgress,
      kReady,
    };

    union Payload {
      T obj;

      Payload() noexcept {
      }

      ~Payload() {
      }
    };

    Payload payload_;
    std::atomic<Status> status_;
  };

  export template<>
  class Optional<void> {
  public:
    bool TryPush() noexcept {
      bool exp = false;
      if (!status_.compare_exchange_strong(exp, true)) {
        return false;
      }
      status_.notify_one();
      return true;
    }

    bool TryPop() noexcept {
      bool exp = true;
      if (!status_.compare_exchange_strong(exp, false)) {
        return false;
      }
      status_.notify_one();
      return true;
    }

    void PushUnsafe() noexcept {
      status_.store(true);
      status_.notify_one();
    }

    void PopUnsafe() noexcept {
      status_.store(false);
      status_.notify_one();
    }

    void Push() noexcept {
      bool exp = false;
      while (!status_.compare_exchange_strong(exp, true)) {
        status_.wait(exp, std::memory_order_relaxed);
        exp = false;
      }
      status_.notify_one();
    }

    void Pop() noexcept {
      bool exp = true;
      while (!status_.compare_exchange_strong(exp, false)) {
        status_.wait(exp, std::memory_order_relaxed);
        exp = true;
      }
      status_.notify_one();
    }

    bool IsReady() noexcept {
      return status_.load();
    }

    bool IsEmpty() noexcept {
      return !status_.load();
    }

  private:
    std::atomic<bool> status_;
  };

} // namespace async
