#include <iostream>
#include <coroutine>

struct Obj {
    ~Obj() {
        std::cerr << "123" << std::endl;
    }

    int x = 1;
};


struct coro {
    struct promise_type {
        std::suspend_always initial_suspend() noexcept {
            return {};
        }
    
        auto final_suspend() noexcept {
            struct custom_suspend : public std::suspend_always {
                void await_suspend(std::coroutine_handle<> handle) noexcept {
                    std::cerr << "Zdes\n";
                    handle.destroy();
                }
            };
            return custom_suspend{};
        }
    
        void unhandled_exception() {
            throw;
        }
    
        coro get_return_object() {
            return coro{
                .handle = std::coroutine_handle<promise_type>::from_promise(*this),
                .obj = obj
            };
        }
    
        void return_void() {
        }
    
        Obj obj;
    };

    std::coroutine_handle<> handle;
    Obj& obj;
};

coro foo() {
    co_return;
}

void goo() {
    auto task = foo();
    task.handle.resume();
    // task.handle.destroy();
}

int main() {
    goo();
    return 0;
}
