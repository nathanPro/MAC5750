#include "IRBuilder.h"
#include "gtest/gtest.h"

class IRBuilderTest : public ::testing::Test {
    protected:
        IRBuilderTest() : tree() {}

    IR::Tree tree;
};

TEST_F(IRBuilderTest, buildMethodChangesTree) {
    IRBuilder builder(tree);
    builder << IR::ExpId::CONST << 42;
    EXPECT_EQ(tree.size(), 0);
    builder.build();
    EXPECT_EQ(tree.size(), 1);
}

TEST_F(IRBuilderTest, constBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::CONST << 42;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_const(ref).value, 42);
}

TEST_F(IRBuilderTest, nameBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::NAME << 13;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_name(ref).label, 13);
}

TEST_F(IRBuilderTest, treeKeepsType) {
    int c, n;
    {
        IRBuilder builder(tree);
        builder << IR::ExpId::CONST << 42;
        c = builder.build();
    }
    {
        IRBuilder builder(tree);
        builder << IR::ExpId::NAME << 13;
        n = builder.build();
    }
    EXPECT_EQ(tree.get_type(c), IR::ExpId::CONST);
    EXPECT_EQ(tree.get_type(n), IR::ExpId::NAME);
    EXPECT_NE(tree.get_type(c), IR::ExpId::NAME);
    EXPECT_NE(tree.get_type(n), IR::ExpId::CONST);
}

TEST_F(IRBuilderTest, wrongGetNameThrows) {
    IRBuilder builder(tree);
    builder << IR::ExpId::CONST << 42;
    auto c = builder.build();
    EXPECT_THROW(tree.get_name(c), IR::BadAccess);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
