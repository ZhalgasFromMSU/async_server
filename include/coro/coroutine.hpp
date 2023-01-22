#pragma once

#include <coroutine>

namespace NAsync {

    // This coroutines are like rusts coroutines
    template<typename T>
    class TCoroutine {
    public:
        struct promise_type {
            std::suspend_always initial_suspend() noexcept {
                return {};
            }

            std::suspend_always final_suspend() noexcept {
                return {};
            }

            void unhandled_exception() {
                throw;
            }

            void return_void() {
                return;
            }

            TCoroutine get_return_object() {
                return TCoroutine{*this};
            }
        };

        T RunToCompletion();

    private:
        promise_type Promise_;
    };

}