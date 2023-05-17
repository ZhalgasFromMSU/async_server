#include <http/message.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(Http, SerializeRequest) {
    THttpRequest req;
    std::cerr << req.inspect() << std::endl;
}

