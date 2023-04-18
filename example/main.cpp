#include <iostream>
#include <functional>
#include <variant>
#include "../include/util/result.hpp"

struct TNonCopy {
    TNonCopy(int x) : X{x} {}
    TNonCopy(const TNonCopy&) = delete;
    TNonCopy& operator=(const TNonCopy&) = delete;

    int X;
};

NAsync::TResult<TNonCopy> foo() {
    return NAsync::TResult<TNonCopy>{10};
}


int main() {
    auto res = foo();
    return 0;
}
