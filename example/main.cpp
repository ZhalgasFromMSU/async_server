#include <iostream>
#include <array>
#include <atomic>


template<typename T>
struct A {
    A(int, T);
};

struct B : A<const char*> {
    using B::A::A;
};


int main() {
    B b{1, 2};

    return 0;
}
