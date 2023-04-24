#include <iostream>
#include <memory>
#include <atomic>


template<typename T>
struct TStruct {
    T Val;
    std::atomic<TStruct*> Next;
};

int main() {
    std::atomic<TStruct<int>*> a = new TStruct<int> {
        .Val = 3,
        .Next = nullptr,
    };

    std::atomic<TStruct<int>*> b = new TStruct<int> {
        .Val = 4,
        .Next = nullptr,
    };

    auto copy = a.load();
    while (!a->Next.compare_exchange_weak(nullptr, b)) {

    }
    std::cerr << (*a).Val << std::endl;
    std::cerr << (*((*a).Next)).Val << std::endl;

    return 0;
}
