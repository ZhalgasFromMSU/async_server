#pragma once

#include <variant>
#include <functional>
#include <coroutine>

namespace NAsync {

    template<typename TFunc>
    concept CVoidToVoid = std::is_same_v<void, std::invoke_result_t<TFunc>>;

    class TJob : public std::variant<std::coroutine_handle<>,
                                     std::function<void()>> {
    public:

        template<std::derived_from<std::coroutine_handle<>> TCoroHandle>
        TJob(TCoroHandle&& job)
            : TJob::variant{std::in_place_type<std::coroutine_handle<>>, std::forward<TCoroHandle>(job)}
        {}

        template<typename TFunc>
        TJob(TFunc&& job)
            : TJob::variant(std::in_place_type<std::function<void()>>, std::forward<TFunc>(job))
        {}

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
