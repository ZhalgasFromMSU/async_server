#include <socket/resolve.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

void CheckSocket(const TSockDescr& descr, const std::string& ipStr, uint16_t port, EDomain domain, ESockType type) {
    ASSERT_EQ(descr.Domain(), domain);
    ASSERT_EQ(descr.Port(), port);
    ASSERT_EQ(descr.Type(), type);
    ASSERT_EQ(descr.StrAddr(), ipStr);
}

TEST(Resolve, Localhost) {
    auto ret = TResolver::ResolveSync("localhost", "1234", EDomain::kIPv4, ESockType::kTcp);
    ASSERT_TRUE(ret) << ret.Error().message();
    ASSERT_EQ(ret->size(), 1);
    CheckSocket(ret->at(0), "127.0.0.1", 1234, EDomain::kIPv4, ESockType::kTcp);
}

TEST(Resolve, AnyAddr) {
    {
        auto ret = TResolver::ResolveSync(nullptr, "1234", EDomain::kIPv6, ESockType::kUdp);
        ASSERT_TRUE(ret) << ret.Error().message();
        CheckSocket(ret->at(0), "::", 1234, EDomain::kIPv6, ESockType::kUdp);
    }

    {
        auto ret = TResolver::ResolveSync(nullptr, "1234", EDomain::kIPv4, ESockType::kUdp);
        ASSERT_TRUE(ret) << ret.Error().message();
        CheckSocket(ret->at(0), "0.0.0.0", 1234, EDomain::kIPv4, ESockType::kUdp);
    }
}
