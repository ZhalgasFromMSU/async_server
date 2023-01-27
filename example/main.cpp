#include <iostream>
#include <tuple>
#include <type_traits>


struct Base {
    virtual Base Foo() = 0;
};

struct Derived: Base {
    Derived Foo() override {
        std::cerr << "foo" << std::endl;
        return Derived();
    }
};


int main() {
    Derived d;
    d.Foo();
    return 0;
}