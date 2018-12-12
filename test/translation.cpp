#include "error.h"
#include "parser.h"
#include "translate.h"
#include "gtest/gtest.h"
#include <sstream>

TEST(translatorTest, translateSum)
{
    IR::Tree tree;
    auto     stream = std::stringstream("3 + 4\n");
    auto     ir     = IR::translate(tree, Parser(&stream).Exp());
    ASSERT_NE(ir, -1);
    EXPECT_EQ(tree.get_type(ir), IR::IRTag::BINOP);
    auto [op, lhs, rhs] = tree.get_binop(ir);
    EXPECT_EQ(op, IR::BinopId::PLUS);
    ASSERT_NE(lhs, -1);
    ASSERT_NE(rhs, -1);
    EXPECT_EQ(tree.get_type(lhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_type(rhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_const(lhs).value, 3);
    EXPECT_EQ(tree.get_const(rhs).value, 4);
}

TEST(translatorTest, translateProd)
{
    IR::Tree tree;
    auto     stream = std::stringstream("3 * (4 + 2)\n");
    auto     ir     = IR::translate(tree, Parser(&stream).Exp());
    ASSERT_NE(ir, -1);
    EXPECT_EQ(tree.get_type(ir), IR::IRTag::BINOP);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
