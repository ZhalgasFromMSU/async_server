#include <iostream>
#include "src/io/io_reader.hpp"

using namespace NAsync;

int main() {
    std::stringstream ss;

    while (TReader{0} >> ss != EReadResult::kReachedEof) {

    }

    return 0;
}