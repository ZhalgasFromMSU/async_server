#include <iostream>


void foo(int&& i) {
    std::cerr << "&&\n";
}

void foo(const int& i) {
    std::cerr << "const&\n";
}

template<typename T>
struct A {
    const T& t;
};


int main() {
    int x;
    A<int> a{.t = std::move(x)};
    return 0;
}