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

TEST_F(IRBuilderTest, tempBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::TEMP << 11;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_temp(ref).id, 11);
}

TEST_F(IRBuilderTest, binopBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::BINOP
            << IR::BinOp::PLUS << 10 << 11;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_binop(ref).op, IR::BinOp::PLUS);
    EXPECT_EQ(tree.get_binop(ref).lhs, 10);
    EXPECT_EQ(tree.get_binop(ref).rhs, 11);
}

TEST_F(IRBuilderTest, memBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::MEM
            << 32;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_mem(ref).exp, 32);
}

TEST_F(IRBuilderTest, callBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::CALL
            << 1 << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_call(ref).func, 1);
    EXPECT_EQ(tree.get_call(ref).explist, 2);
}

TEST_F(IRBuilderTest, eseqBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::ESEQ
            << 1 << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_eseq(ref).stm, 1);
    EXPECT_EQ(tree.get_eseq(ref).exp, 2);
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
