#include <iostream>


struct A {
    int x;
};


struct B: A {
};


void foo(B* b) {
    std::cerr << b << std::endl;
    delete b;
}

int main() {
    A* a = new A;
    std::cerr << a << std::endl;

    foo(static_cast<B*>(a));

    return 0;
}