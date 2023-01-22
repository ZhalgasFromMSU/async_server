#pragma once

#include <coroutine>
#include <future>

namespace NAsync {

    class TPromiseBase {
    public:
        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {
            throw;
        }
    };

    template<typename T>
    class TPromise {
    public:
        void return_value(T ret) {
            RetPromise_.set_value(std::move(ret));
            return;
        }

        TCoroutine get_return_object() {
            return TCoroutine{*this};
        }

    private:
        std::promise<T> RetPromise_;
    };

    template<>
    class TPromise<void> {
    public:
        void return_void() {
            RetPromise_.set_value();
        }

    private:
        std::promise<void> RetPromise_;
    }

}