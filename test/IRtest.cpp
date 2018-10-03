#include "IRBuilder.h"
#include "gtest/gtest.h"

class IRTest : public ::testing::Test {
    protected:
        IRTest() : tree() {}

    IR::Tree tree;
};

TEST_F(IRTest, BuilderFromTree) {
    IRBuilder builder(tree);
    builder << IR::ExpId::CONST << 42;
    EXPECT_EQ(tree.size(), 0);
    builder.build();
    EXPECT_EQ(tree.size(), 1);
}

TEST_F(IRTest, BuilderConst) {
    IRBuilder builder(tree);
    builder << IR::ExpId::CONST << 42;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_const(ref).value, 42);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
