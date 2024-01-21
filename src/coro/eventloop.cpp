module;

#include <coroutine>

export module async:eventloop;
import :operation;

namespace async {

  export template<typename IoDispatcher>
  class EventLoop {
  public:
    bool await_ready() noexcept;
    void await_suspend() noexcept;
    OpResult await_resume() noexcept;

  private:
    IoDispatcher& dispatcher_;
  };

} // namespace async
