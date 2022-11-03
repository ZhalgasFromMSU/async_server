#pragma once

#include <sstream>

#include "base.hpp"

namespace NAsync {

    enum class EReadResult {
        kReachedEof,
        kNotReady, // only applicable to non-blocking descriptors. Means that not ready to read
        kReadChunk,
    };

    class TReader {
    public:
        TReader(int fd);
        TReader(const IIoObject& ioObject);
        TReader(const TReader&) = delete;

        // these overloads return false if EOF reached
        template<typename T>
        EReadResult operator>>(T& output) && noexcept {
            std::stringstream tmp;  // extra allocations
            EReadResult haveMoreData = (std::move(*this) >> tmp);
            tmp >> output;
            return haveMoreData;
        }

        EReadResult operator>>(std::stringstream& output) && noexcept;

    private:
        static constexpr size_t BUFF_SIZE = 8192;

        int Fd_;
    };

} // namespace NAsync