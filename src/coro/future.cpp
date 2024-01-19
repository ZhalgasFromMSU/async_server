module;

#include <coroutine>
#include <exception>
#include <iostream>

export module async:future;
import :optional;

namespace async {

  export template<typename T>
  class Future {
  public:
    struct promise_type {
      std::suspend_always initial_suspend() noexcept {
        return {};
      }

      std::suspend_always final_suspend() noexcept {
        return {};
      }

      Future get_return_object() noexcept {
        return Future{*this};
      }

      void unhandled_exception() noexcept {
        try {
          std::rethrow_exception(std::current_exception());
        } catch (const std::exception& e) {
          std::cerr << e.what() << std::endl;
          std::terminate();
        }
      }

      void return_void() noexcept {
        static_assert(std::is_same_v<T, void>);
        payload.EmplaceUnsafe();
      }

      template<typename U>
      void return_value(U&& ret) noexcept {
        payload.EmplaceUnsafe(std::forward<U>(ret));
      }

      Optional<T> payload;
    };

  private:
    explicit Future(promise_type& promise) noexcept
        : promise_{promise} {
    }

    promise_type& promise_;
  };

} // namespace async
