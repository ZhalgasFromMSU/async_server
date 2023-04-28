#pragma once

#include <variant>
#include <functional>
#include <coroutine>

namespace NAsync {

    class TJob : public std::variant<std::coroutine_handle<>,
                                     std::function<void()>> {
    public:
        using TJob::variant::variant;

        void Execute() {
            if (auto handle = std::get_if<std::coroutine_handle<>>(this)) {
                handle->resume();
            } else {
                auto& func = std::get<std::function<void()>>(*this);
                func();
            }
        }
    };
} // namespace NAsync
