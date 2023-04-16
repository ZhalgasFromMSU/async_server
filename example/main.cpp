#include <iostream>
#include <functional>

int foo(int x, int y) {
    return x + y;
}

int main() {
    auto f = std::bind(foo, 1, 2);
    std::cerr << f() << std::endl;
    return 0;
}