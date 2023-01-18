#include <io/pipe.hpp>

#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>

TEST(IoObject, ReadWrite) {
    NAsync::TIoObject ioObject(eventfd(0, EFD_NONBLOCK));
    uint64_t val = 10;
    auto result1 = ioObject.Write(&val, sizeof(val));
    VERIFY_RESULT(result1);
    ASSERT_EQ(*result1, sizeof(val));

    uint64_t valToRead = 0;
    auto result2 = ioObject.Read(&valToRead, sizeof(valToRead));
    VERIFY_RESULT(result2);
    ASSERT_EQ(*result2, sizeof(valToRead));
    ASSERT_EQ(valToRead, val);

    auto result3 = ioObject.Read(&valToRead, sizeof(valToRead));
    ASSERT_TRUE(result3.Error().value() == EAGAIN || result3.Error().value() == EWOULDBLOCK);

    NAsync::TIoObject ioObjectCopy = std::move(ioObject);
}

TEST(IoObject, PipeReadWrite) {
    auto [readEnd, writeEnd] = NAsync::CreatePipe();

    const char strToWrite[] = "12345";
    auto writeResult = writeEnd.Write(strToWrite, sizeof(strToWrite));
    VERIFY_RESULT(writeResult);
    ASSERT_EQ(*writeResult, sizeof(strToWrite));

    constexpr int readBufSize = sizeof(strToWrite) * 3;
    char readBuf[readBufSize];
    auto readResult = readEnd.Read(readBuf, readBufSize);
    VERIFY_RESULT(readResult);
    ASSERT_EQ(*readResult, sizeof(strToWrite));
}
