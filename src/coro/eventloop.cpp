module;

#include <cassert>
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
      StopWorker();
    }

    void StartWorker() noexcept {
      worker_ = std::jthread{[this] {
        while (true) {
          auto mb_awaitable = dispatcher_.Dequeue();
          if (!mb_awaitable) {
            assert(!mb_awaitable.error() && "Dispatcher returned error");
            break;
          }

          (*mb_awaitable)->ResumeCoro();
        }
      }};
    }

    void StopWorker() noexcept {
      dispatcher_.DispatchStop();
    }

  private:
    IoDispatcher& dispatcher_;
    std::jthread worker_;
  };

} // namespace async
