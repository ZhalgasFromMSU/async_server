#include <iostream>

struct A;

struct B {
    B(A a) {
        a.foo();
    }
};

struct A {
    void foo() {
        std::cerr << "Zdes\n";
    }
};

int main() {
    B b{A()}w;
    return 0;
}