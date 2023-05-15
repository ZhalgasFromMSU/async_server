#include <iostream>
#include <memory>

struct A {
    const std::unique_ptr<int> i;
};


A foo() {
    return A{};
}

int main() {
    A a = foo();
    A b = std::move(a);
    return 0;
}
