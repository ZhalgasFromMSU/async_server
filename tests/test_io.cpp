#include <io/well_known_structs.hpp>
#include <io/pollable_object.hpp>

#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(IoObject, ReadWrite) {

    TIoObject ioObject(eventfd(0, EFD_NONBLOCK));

    uint64_t val = 10;
    auto result1 = Write(ioObject, &val, sizeof(val));
    VERIFY_RESULT(result1);
    ASSERT_EQ(*result1, sizeof(val));

    uint64_t valToRead = 0;
    auto result2 = Read(ioObject, &valToRead, sizeof(valToRead));
    VERIFY_RESULT(result2);
    ASSERT_EQ(*result2, sizeof(valToRead));
    ASSERT_EQ(valToRead, val);

    auto result3 = Read(ioObject, &valToRead, sizeof(valToRead));
    ASSERT_TRUE(result3.Error().value() == EAGAIN || result3.Error().value() == EWOULDBLOCK);
}

TEST(IoObject, PipeReadWrite) {
    auto pipe = TPipe::Create();

    const char strToWrite[] = "12345";
    auto writeResult = Write(pipe.WriteEnd(), strToWrite, sizeof(strToWrite));
    VERIFY_RESULT(writeResult);
    ASSERT_EQ(*writeResult, sizeof(strToWrite));

    constexpr int readBufSize = sizeof(strToWrite) * 3;
    char readBuf[readBufSize];
    auto readResult = Read(pipe.ReadEnd(), readBuf, readBufSize);
    VERIFY_RESULT(readResult);
    ASSERT_EQ(strcmp(readBuf, strToWrite), 0);
    ASSERT_EQ(*readResult, sizeof(strToWrite));
}

TEST(IoObject, PollableObject) {
    auto pipe = TPipe::Create();
    char buffer[10];
    TReadPollable read{pipe.ReadEnd(), buffer, sizeof(buffer)};
    ASSERT_EQ(read.Try(), std::nullopt);
    TWritePollable write{pipe.WriteEnd(), "1234", 4};
    ASSERT_EQ(**write.Try(), 4);
    ASSERT_EQ(**read.Try(), 4);
}
