#pragma once

#include <ostream>

#include "io_base.hpp"

namespace NAsync {

    enum class EReadResult {
        kReachedEof,
        kReachedBlock, // only applicable to non-blocking descriptors. Means that not ready to read
        kReadChunk,
    };

    class TReader {
    public:
        TReader(int fd);
        TReader(const IIoObject& ioObject);
        TReader(const TReader&) = delete;

        EReadResult ReadChunkInto(std::ostream& output) && noexcept;

        // Read until EOF or until block unblocking fd
        EReadResult ReadInto(std::ostream& output) && noexcept;

    private:
        static constexpr size_t BuffSize_ = 8192;

        int Fd_;
    };

} // namespace NAsync