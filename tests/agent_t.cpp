#include "agent/agent.hpp"
#include <gtest/gtest.h>

using namespace agent;

TEST(add_one, sample)
{
  EXPECT_EQ(add_one(0), 1);
  EXPECT_EQ(add_one(123), 124);
  EXPECT_EQ(add_one(-1), 0);
}
