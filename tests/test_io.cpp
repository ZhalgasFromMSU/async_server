#include <io/well_known_structs.hpp>
#include <io/read_write_awaitable.hpp>

#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(IoObject, ReadWrite) {

    TIoObject ioObject(eventfd(0, EFD_NONBLOCK));

    uint64_t val = 10;
    auto result1 = ioObject.Write(&val, sizeof(val)).await_resume();
    VERIFY_RESULT(result1);
    ASSERT_EQ(*result1, sizeof(val));

    uint64_t valToRead = 0;
    auto result2 = ioObject.Read(&valToRead, sizeof(valToRead)).await_resume();
    VERIFY_RESULT(result2);
    ASSERT_EQ(*result2, sizeof(valToRead));
    ASSERT_EQ(valToRead, val);

    auto result3 = ioObject.Read(&valToRead, sizeof(valToRead)).await_resume();
    ASSERT_TRUE(result3.Error().value() == EAGAIN || result3.Error().value() == EWOULDBLOCK);
}

TEST(IoObject, PipeReadWrite) {
    TPipe pipe;

    const char strToWrite[] = "12345";
    auto writeResult = pipe.WriteEnd().Write(strToWrite, sizeof(strToWrite)).await_resume();
    VERIFY_RESULT(writeResult);
    ASSERT_EQ(*writeResult, sizeof(strToWrite));

    constexpr int readBufSize = sizeof(strToWrite) * 3;
    char readBuf[readBufSize];
    auto readResult = pipe.ReadEnd().Read(readBuf, readBufSize).await_resume();
    VERIFY_RESULT(readResult);
    ASSERT_EQ(strcmp(readBuf, strToWrite), 0);
    ASSERT_EQ(*readResult, sizeof(strToWrite));
}

TEST(IoObject, PollableObject) {
    TPipe pipe;
    char buffer[10];
    TReadAwaitable read{pipe.ReadEnd(), buffer, sizeof(buffer)};
    ASSERT_FALSE(read.await_resume());
    TWriteAwaitable write{pipe.WriteEnd(), "1234", 4};
    ASSERT_EQ(*write.await_resume(), 4);
    ASSERT_EQ(*read.await_resume(), 4);
}
