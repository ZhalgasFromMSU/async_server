#include <iostream>
#include <sstream>

#include <io/io_reader.hpp>


int main() {
    std::stringstream ss;
    NAsync::TReader{0}.ReadInto(ss);
    std::cout << ss.str() << std::endl;
    return 0;
}