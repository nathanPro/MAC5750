#include "IRBuilder.h"
#include "gtest/gtest.h"

class IRTest : public ::testing::Test {
    protected:
        IRTest() : tree() {}

    IR tree;
};

TEST_F(IRTest, BuilderFromTree) {
    IRBuilder builder(tree);
    EXPECT_EQ(tree.size(), 0);
    builder << IR::Exp::Const << 42;
    EXPECT_EQ(tree.size(), 1);
}
