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
    builder << IR::ExpId::BINOP << IR::BinopId::PLUS << 10 << 11;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_binop(ref).op, IR::BinopId::PLUS);
    EXPECT_EQ(tree.get_binop(ref).lhs, 10);
    EXPECT_EQ(tree.get_binop(ref).rhs, 11);
}

TEST_F(IRBuilderTest, memBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::MEM << 32;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_mem(ref).exp, 32);
}

TEST_F(IRBuilderTest, callBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::CALL << 1 << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_call(ref).func, 1);
    EXPECT_EQ(tree.get_call(ref).explist, 2);
}

TEST_F(IRBuilderTest, eseqBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::ExpId::ESEQ << 1 << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_eseq(ref).stm, 1);
    EXPECT_EQ(tree.get_eseq(ref).exp, 2);
}

TEST_F(IRBuilderTest, moveBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::StmId::MOVE << 1 << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_move(ref).dst, 1);
    EXPECT_EQ(tree.get_move(ref).src, 2);
}

TEST_F(IRBuilderTest, expBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::StmId::EXP << 1;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_exp(ref).exp, 1);
}

TEST_F(IRBuilderTest, jumpBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::StmId::JUMP << 1 << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_jump(ref).exp, 1);
    EXPECT_EQ(tree.get_jump(ref).targets, 2);
}

TEST_F(IRBuilderTest, cjumpBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::StmId::CJUMP << IR::RelopId::LE << 2 << 3 << 32
            << 36;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_cjump(ref).op, IR::RelopId::LE);
    EXPECT_EQ(tree.get_cjump(ref).lhs, 2);
    EXPECT_EQ(tree.get_cjump(ref).rhs, 2);
    EXPECT_EQ(tree.get_cjump(ref).iftrue, 32);
    EXPECT_EQ(tree.get_cjump(ref).iffalse, 36);
}

TEST_F(IRBuilderTest, seqBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::StmId::SEQ << 13 << 15;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_seq(ref).lhs, 13);
    EXPECT_EQ(tree.get_seq(ref).rhs, 15);
}

TEST_F(IRBuilderTest, labelBuiltWithBuilder) {
    IRBuilder builder(tree);
    builder << IR::StmId::LABEL << 12;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_label(ref).label, 12);
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

TEST_F(IRBuilderTest, stmAccessedAsExpThrows) {
    IRBuilder builder(tree);
    builder << IR::StmId::MOVE << 12 << 14;
    auto stm = builder.build();
    EXPECT_EQ(IR::ExpId::CONST, IR::StmId::MOVE);
    EXPECT_THROW(tree.get_const(stm), IR::BadAccess);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
