#include <socket/resolve.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(Socket, Resolve) {
    auto ret = TResolver::ResolveSync("localhost", "83", {}, ESockType::kUdp);
    ASSERT_TRUE(ret) << ret.Error().message();

    for (const auto& er : *ret) {
        std::cerr << er.StrAddr() << '\t' << er.Port() << std::endl;
    }
}
