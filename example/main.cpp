#include <iostream>
#include "tmp.hpp"
#include "tmp2.hpp"


int main() {
    TFakeStruct a;
    std::cerr << Foo(a).a << std::endl;
    return 0;
}