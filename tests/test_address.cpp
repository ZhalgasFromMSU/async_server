#include <net/address/ip.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(Address, Trivial) {
    ASSERT_EQ(TIPv4Address::Localhost().ToString(), "127.0.0.1");
    ASSERT_EQ(TIPv4Address::Any().ToString(), "0.0.0.0");

    ASSERT_EQ(TIPv6Address::Localhost().ToString(), "::1");
    ASSERT_EQ(TIPv6Address::Any().ToString(), "::");

    ASSERT_EQ(TIPv6Address::FromString("::FFF:127.0.0.1")->ToString(), "::fff:7f00:1");
}
