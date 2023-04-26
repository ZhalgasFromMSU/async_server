#include <iostream>
#include <array>
#include <atomic>


void foo(const int& x) {
    std::cerr << "c&\n";
}

void foo(int&& x) {
    std::cerr << "&&\n";
}

template<typename T>
struct A {

    template<typename TVal>
    void Foo(TVal&& t) {
        foo(std::forward<T>(t));
    }
};


int main() {
    A<int> a;

    int x;
    a.Foo(x);
    a.Foo(std::move(x));

    return 0;
}
