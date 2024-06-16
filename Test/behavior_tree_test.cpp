#ifndef _TEST_BEHAVIORTREE
#define _TEST_BEHAVIORTREE

#include "gtest/gtest.h"
#include "../Engine/behavior_tree.h"

namespace UnitTest {

TEST(BehaviorTreeBuilder, BehaviorTreeBuilder) {

    Engine::BehaviorTreeBuilder builder;
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}
} // namespace UnitTest
#endif