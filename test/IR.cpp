#include "IRBuilder.h"
#include "gtest/gtest.h"

class IRBuilderTest : public ::testing::Test
{
  protected:
    IRBuilderTest() : tree() {}

    IR::Tree tree;
};

TEST_F(IRBuilderTest, buildMethodChangesTree)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::CONST << 42;
    EXPECT_EQ(tree.size(), 0);
    builder.build();
    EXPECT_EQ(tree.size(), 1);
}

TEST_F(IRBuilderTest, constBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::CONST << 42;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_const(ref).value, 42);
}

TEST_F(IRBuilderTest, nameBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::REG << 13;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_reg(ref).id, 13);
}

TEST_F(IRBuilderTest, tempBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::TEMP;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_temp(ref).id, 0);
}

TEST_F(IRBuilderTest, binopBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::BINOP << IR::BinopId::PLUS << 10 << 11;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_binop(ref).op, IR::BinopId::PLUS);
    EXPECT_EQ(tree.get_binop(ref).lhs, 10);
    EXPECT_EQ(tree.get_binop(ref).rhs, 11);
}

TEST_F(IRBuilderTest, memBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::MEM << 32;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_mem(ref).exp, 32);
}

TEST_F(IRBuilderTest, callBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::CALL << std::string("fn") << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_call(ref).fn, std::string("fn"));
    EXPECT_EQ(tree.get_call(ref).explist, 2);
}

TEST_F(IRBuilderTest, moveBuiltWithBuilder)
{
    IRBuilder builder(tree);
    auto      t1 = tree.new_temp();
    auto      t2 = tree.new_temp();
    builder << IR::IRTag::MOVE << t1 << t2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_move(ref).dst, t1);
    EXPECT_EQ(tree.get_move(ref).src, t2);
    EXPECT_EQ(tree.stm_seq.back(), ref);
}

TEST_F(IRBuilderTest, moveCoercesLHS)
{
    IRBuilder builder(tree);
    auto      tmp = tree.new_temp();
    builder << IR::IRTag::MOVE << [&] {
        IRBuilder cte(tree);
        cte << IR::IRTag::CONST << 0;
        return cte.build();
    }() << tmp;

    auto ref = builder.build();
    EXPECT_EQ(tree.get_type(tree.get_move(ref).dst), IR::IRTag::MEM);
    EXPECT_EQ(tree.get_move(ref).src, tmp);
    EXPECT_EQ(tree.stm_seq.back(), ref);
}

TEST_F(IRBuilderTest, expBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::EXP << 1;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_exp(ref).exp, 1);
    EXPECT_EQ(tree.stm_seq.back(), ref);
}

TEST_F(IRBuilderTest, jmpBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::JMP << 2;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_jmp(ref).target, 2);
    EXPECT_EQ(tree.stm_seq.back(), ref);
}

TEST_F(IRBuilderTest, labelBuilt)
{
    auto handle = tree.new_label();
    EXPECT_EQ(tree.stm_seq.size(), 0);
    tree.place_label(std::move(handle));
    EXPECT_EQ(tree.stm_seq.size(), 1);
    tree.place_label(tree.new_label());
    EXPECT_EQ(tree.stm_seq.size(), 2);
}

TEST_F(IRBuilderTest, cmpBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::CMP << 12 << 13;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_cmp(ref).lhs, 12);
    EXPECT_EQ(tree.get_cmp(ref).rhs, 13);
}

TEST_F(IRBuilderTest, cjmpBuiltWithBuilder)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::CJMP << 11 << 12;
    auto ref = builder.build();
    EXPECT_EQ(tree.get_cjmp(ref).temp, 11);
    EXPECT_EQ(tree.get_cjmp(ref).target, 12);
    EXPECT_EQ(tree.stm_seq.back(), ref);
}

TEST_F(IRBuilderTest, treeKeepsType)
{
    int c, n;
    {
        IRBuilder builder(tree);
        builder << IR::IRTag::CONST << 42;
        c = builder.build();
    }
    {
        IRBuilder builder(tree);
        builder << IR::IRTag::REG << 13;
        n = builder.build();
    }
    EXPECT_EQ(tree.get_type(c), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_type(n), IR::IRTag::REG);
    EXPECT_NE(tree.get_type(c), IR::IRTag::REG);
    EXPECT_NE(tree.get_type(n), IR::IRTag::CONST);
}

TEST_F(IRBuilderTest, wrongGetNameThrows)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::CONST << 42;
    auto c = builder.build();
    EXPECT_THROW(tree.get_reg(c), IR::BadAccess);
}

TEST_F(IRBuilderTest, stmAccessedAsExpThrows)
{
    IRBuilder builder(tree);
    builder << IR::IRTag::MOVE << 12 << 14;
    auto stm = builder.build();
    EXPECT_THROW(tree.get_const(stm), IR::BadAccess);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
