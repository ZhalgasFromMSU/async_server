module;

#include <coroutine>

export module async:promise;

namespace async {

  template<typename T>
  class Promise {
  public:
    std::suspend_always initial_suspend() noexcept {
      return {};
    }

    std::suspend_always final_suspend() noexcept {
      return {};
    }

    void return_void() {
    }

  private:
  };

} // namespace async
