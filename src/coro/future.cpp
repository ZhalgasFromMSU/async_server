module;

#include <coroutine>
#include <exception>
#include <iostream>

export module async:future;
import :optional;

namespace async {

  export template<typename T>
  class Future;

  template<typename T>
  struct PromiseBase {
    std::suspend_always initial_suspend() noexcept {
      return {};
    }

    std::suspend_always final_suspend() noexcept {
      return {};
    }

    void unhandled_exception() noexcept {
      try {
        std::rethrow_exception(std::current_exception());
      } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::terminate();
      }
    }

    Future<T> get_return_object() noexcept;

    Optional<T> payload;
  };

  template<typename T>
  struct Promise : PromiseBase<T> {
    template<typename U>
    void return_value(U&& ret) noexcept {
      this->payload.Push(std::forward<U>(ret));
    }
  };

  template<>
  struct Promise<void> : PromiseBase<void> {
    void return_void() noexcept {
      payload.Push();
    }
  };

  template<typename T>
  class Future {
  public:
    using promise_type = Promise<T>;

    Future& Run() noexcept {
      std::coroutine_handle<promise_type>::from_promise(promise_).resume();
      return *this;
    }

    T Get() noexcept {
      return promise_.payload.Pop();
    }

  private:
    friend PromiseBase<T>;

    explicit Future(promise_type& promise) noexcept
        : promise_{promise} {
    }

    promise_type& promise_;
  };

  template<typename T>
  Future<T> PromiseBase<T>::get_return_object() noexcept {
    return Future<T>{static_cast<Promise<T>&>(*this)};
  }

} // namespace async
