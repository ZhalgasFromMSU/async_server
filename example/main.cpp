#include <experimental/coroutine>


int main() {
    std::experimental::coroutine_handle<> s;
    std::cerr << bool(s) << std::endl;
    return 0;
}