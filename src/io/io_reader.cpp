#include "io_reader.hpp"

#include <unistd.h>
#include <errno.h>

namespace NAsync {

    namespace {
        ssize_t ReadToBuffer(int fd, char* buffer, size_t nBytes) noexcept {
            ssize_t count = read(fd, buffer, nBytes);
            if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                std::terminate();
            }
            return count;
        }
    }

    TReader::TReader(int fd)
        : Fd_(fd)
    {}

    TReader::TReader(const IIoObject& ioObject)
        : Fd_(ioObject.GetFd())
    {}


    EReadResult TReader::ReadChunkInto(std::ostream& output) && noexcept {
        char buffer[BuffSize_];
        ssize_t count = ReadToBuffer(Fd_, buffer, BuffSize_);
        if (count < 0) {
            return EReadResult::kReachedBlock;
        } else if (count == 0) {
            return EReadResult::kReachedEof;
        } else {
            output.write(buffer, count);
            return EReadResult::kReadChunk;
        }
    }

    EReadResult TReader::ReadInto(std::ostream& output) && noexcept {
        char buffer[BuffSize_];
        ssize_t count;
        do {
            count = ReadToBuffer(Fd_, buffer, BuffSize_);
            if (count < 0) {
                return EReadResult::kReachedBlock;
            } else if (count == 0) {
                return EReadResult::kReachedEof;
            }
            output.write(buffer, count);
        } while (true);
    }

} // namespace NAsync