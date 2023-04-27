#include <iostream>
#include <array>
#include <atomic>



int main() {
    std::array<std::atomic<int>, 1000000> x;
    for (auto& i : x) {
        if (i != 0) {
            std::cerr << "zdes\n";
        }
    }

    return 0;
}
