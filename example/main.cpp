#include <iostream>
#include <array>
#include <atomic>


void foo() {

}

void goo() {
    return foo();
}


int main() {
    goo();
    return 0;
}
