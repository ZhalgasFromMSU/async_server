#include <iostream>
#include <thread>


struct A {

    void Foo() {
        T = std::thread{[this] {
            std::cerr << this << std::endl;
        }};
    };

    std::thread T;
};

int main() {

    return 0;
}
