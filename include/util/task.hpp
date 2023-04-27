#pragma once

#include "result.hpp"

#include <type_traits>
#include <variant>
#include <functional>
#include <coroutine>

namespace NAsync {

    template<typename TFunc>
    concept CVoidToVoid = std::is_same_v<void, std::invoke_result_t<TFunc>>;

    class ITask {
    public:
        virtual ~ITask() = default;

        virtual void Execute() = 0;
    };

    template<CVoidToVoid TFunc>
    class TTask: public ITask {
    public:
        TTask(TFunc&& func)
            : Func_(std::forward<TFunc>(func))
        {}

        void Execute() override {
            Func_();
        }

    private:
        TFunc Func_;
    };

    class TJob : public std::variant<std::coroutine_handle<>,
                                     std::function<void()>> {
    public:
        using TJob::variant::variant;

        void Execute() {
            if (auto handle = std::get_if<std::coroutine_handle<>>(this)) {
                handle->resume();
            } else if (auto handle = std::get_if<std::function<void()>>(this)) {
                (*handle)();
            } else {
                VERIFY(false);
            }
        }
    };
} // namespace NAsync
