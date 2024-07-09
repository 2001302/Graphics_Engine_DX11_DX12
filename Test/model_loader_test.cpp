#ifndef _TEST_MODELLOADER
#define _TEST_MODELLOADER

#include "gtest/gtest.h"
#include "../Engine/model_loader.h"

namespace UnitTest {

TEST(ModelLoader, ModelLoader) {

    Engine::ModelLoader loader;
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}
} // namespace UnitTest

#endif
