module;

#include <coroutine>
#include <iostream>
#include <system_error>

#include <cassert>
#include <unistd.h>

export module async:awaitable;
import :operation;

namespace async {

  export template<typename T>
  concept Executable = requires(T t) {
    { t.Execute() } noexcept -> std::same_as<int>;
  };

  export template<typename T>
  concept Awaitable = Executable<T> && requires(T t) {
    { t.await_ready() } noexcept -> std::same_as<bool>;
    { t.await_suspend({}) } noexcept -> std::same_as<void>;
    { t.await_resume() } noexcept -> std::same_as<int>;
  };

  export class AwaitableBase {
  public:
    virtual ~AwaitableBase() = default;

    virtual int Execute() noexcept = 0;

    void SetResult(int result) noexcept {
      result_ = result;
    }

    int Result() const noexcept {
      return result_;
    }

    void ResumeCoro() noexcept {
      handle_.resume();
    }

  protected:
    std::coroutine_handle<> handle_;
    int result_;
  };

  export template<typename Operation>
  class OperationExecutable : public AwaitableBase {
  public:
    explicit OperationExecutable(Operation&& operation) noexcept
        : operation_{std::move(operation)} {
    }

    Operation& GetOp() noexcept {
      return operation_;
    }

    int Execute() noexcept override {
      int res;
      if constexpr (Operation::op_type == OpType::kRead) {
        res = read(GetArg<arg::fd>(operation_), GetArg<arg::buffer>(operation_),
                   GetArg<arg::count>(operation_));
      } else if constexpr (Operation::op_type == OpType::kWrite) {
        res =
            write(GetArg<arg::fd>(operation_), GetArg<arg::cbuffer>(operation_),
                  GetArg<arg::count>(operation_));
      } else {
        assert(false && "Unreachable");
      }

      if (res >= 0) {
        return res;
      } else {
        return -errno;
      }
    }

  private:
    Operation operation_;
  };

  export template<typename IoProvider, typename Operation>
  class OperationAwaitable final : public OperationExecutable<Operation> {
  public:
    OperationAwaitable(IoProvider& provider, Operation&& operation) noexcept
        : OperationExecutable<Operation>{std::move(operation)}
        , provider_{provider} {
    }

    bool await_ready() noexcept {
      return !provider_.PrepareDispatch(*this);
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
      this->handle_ = handle;
      if (std::error_code err = provider_.Dispatch(*this)) {
        std::cerr << err.message() << std::endl;
        std::terminate();
      }
    }

    int await_resume() noexcept {
      return this->result_;
    }

  private:
    IoProvider& provider_;
  };
} // namespace async
