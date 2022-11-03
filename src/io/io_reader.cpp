#include "io_reader.hpp"

#include <iostream>
#include <unistd.h>
#include <errno.h>

namespace NAsync {

    TReader::TReader(int fd)
        : Fd_(fd)
    {}

    TReader::TReader(const IIoObject& ioObject)
        : Fd_(ioObject.GetFd())
    {}

    EReadResult TReader::operator>>(std::stringstream& output) && noexcept {
        char buffer[BUFF_SIZE + 1]; // +1 for trailing '\0'
        
        ssize_t count;
        do {
            count = read(Fd_, buffer, BUFF_SIZE);
            if (count == 0) {
                return EReadResult::kReachedEof;
            } else if (count < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return EReadResult::kNotReady;
                } else {
                    std::cerr << "Reading from file descriptor failed: " << errno << std::endl;
                    std::terminate();
                }
            } else {
                buffer[count] = 0;
                output << buffer;
            }
        } while (count == BUFF_SIZE);

        return EReadResult::kReadChunk;
    }

} // namespace NAsync