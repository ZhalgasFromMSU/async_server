module;

#include <iostream>
#include <thread>

export module async:eventloop;

namespace async {

  export template<typename IoDispatcher>
  class EventLoop {
  public:
    explicit EventLoop(IoDispatcher& dispatcher) noexcept
        : dispatcher_{dispatcher} {
    }

    ~EventLoop() {
      Stop();
    }

    void Start() noexcept {
      worker_ = std::jthread{[&] {
        while (!stopped_.test(std::memory_order_relaxed)) {
          if (std::error_code err = dispatcher_.Dequeue()) {
            std::cerr << err.message() << std::endl;
            std::terminate();
          }
        }
      }};
    }

    void Stop() noexcept {
      stopped_.test_and_set(std::memory_order_relaxed);
    }

  private:
    IoDispatcher& dispatcher_;
    std::jthread worker_;
    std::atomic_flag stopped_;
  };

} // namespace async
