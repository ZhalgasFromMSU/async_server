#include <iostream>


struct A {

};

struct B: A {

};


int main() {
    std::cerr << sizeof(A) << '\t' << sizeof(B) << std::endl;
    return 0;
}