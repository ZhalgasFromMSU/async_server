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
        std::suspend_never initial_suspend() noexcept {
            return {};
        }
    
        std::suspend_always final_suspend() noexcept {
            std::coroutine_handle<promise_type>::from_promise(*this).destroy();
            return {};
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

int main() {
    auto coro = foo();
    std::cerr << coro.obj.x << std::endl;
    // std::cerr << coro.obj.x << std::endl;
    return 0;
}
