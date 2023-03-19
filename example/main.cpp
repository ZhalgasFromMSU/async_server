#include <iostream>


void foo(int&& i) {
    std::cerr << "&&\n";
}

void foo(const int& i) {
    std::cerr << "const&\n";
}

template<typename T>
void goo(T&& i) {
    i += 1;
    foo(std::forward<T>(i));
}

int main() {
    int i;
    goo<int&>(i);
    goo<int>(std::move(i));
    return 0;
}