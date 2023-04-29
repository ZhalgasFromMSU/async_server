#include <iostream>
#include <coroutine>

struct Obj {
    ~Obj() {

        std::cerr << "123" << std::endl;
    }
};


struct promise {
    std::suspend_never initial_suspend() noexcept {
        std::cerr << "is\n";
        x += 1;
        return {};
    }

    std::suspend_always final_suspend() noexcept {
        std::cerr << "fs " << x << std::endl;
        return {};
    }

    void unhandled_exception() {
        throw;
    }

    std::coroutine_handle<> get_return_object() {
        x += 1;
        return std::coroutine_handle<promise>::from_promise(*this);
    }

    void return_void() {
    }

    int x = 1;
    Obj obj;
};

template<>
struct std::coroutine_traits<std::coroutine_handle<>> {
    using promise_type = promise;
};

std::coroutine_handle<> foo() {
    co_return;
}

int main() {
    auto handle = foo();
    std::cerr << "1\n";
    // handle();
    std::cerr << "2\n";
    handle.destroy();
    // handle();
    //handle();
    //std::cerr << "3\n";
    //handle();
    //std::cerr << "4\n";
    return 0;
}
