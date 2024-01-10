module;

#include <thread>
#include <vector>

export module async:threadpool;

import :queue;
import :waitgroup;

namespace async {

  export template<std::invocable Task>
  class ThreadPool {
  public:
    ThreadPool(std::size_t max_queue_size, std::size_t num_workers) noexcept
        : queue_{max_queue_size, num_workers, num_workers}
        , workers_(num_workers) {
    }

    ~ThreadPool() {
      Stop();
    }

    template<typename... Args>
    bool Enqueue(Args&&... args) {
      if (!wg_.TryInc()) {
        return false;
      }
      queue_.Push(std::forward<Args>(args)...);
      return true;
    }

    void Start() noexcept {
      for (auto& worker : workers_) {
        worker = std::jthread{[&] {
          while (wg_.Dec()) {
            queue_.Pop()();
          }
        }};
      }
    }

    void Stop() noexcept {
      wg_.Block();
      wg_.Wait();
    }

  private:
    Queue<Task> queue_;
    WaitGroup wg_;
    std::vector<std::jthread> workers_;
  };

} // namespace async
