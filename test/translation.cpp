#include "error.h"
#include "helper.h"
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
    EXPECT_EQ(tree.get_type(lhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_const(lhs).value, 3);

    ASSERT_NE(rhs, -1);
    EXPECT_EQ(tree.get_type(rhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_const(rhs).value, 4);
}

TEST(translatorTest, translateProd)
{
    IR::Tree tree;
    auto     stream = std::stringstream("3 * (4 + 2)\n");
    auto     ir     = IR::translate(tree, Parser(&stream).Exp());

    ASSERT_NE(ir, -1);
    EXPECT_EQ(tree.get_type(ir), IR::IRTag::BINOP);
    auto [op, lhs, rhs] = tree.get_binop(ir);
    EXPECT_EQ(op, IR::BinopId::MUL);

    ASSERT_NE(lhs, -1);
    EXPECT_EQ(tree.get_type(lhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_const(lhs).value, 3);

    ASSERT_NE(rhs, -1);
}

TEST(translatorTest, translateMain)
{
    TranslationUnit tu("../input/calc.miniJava");
    ASSERT_TRUE(tu.check());

    IR::Tree tree;
    EXPECT_NO_THROW(tree = IR::Tree(tu));

    std::string const main_class = "Calculator";

    EXPECT_EQ(tu.data.count(main_class), 1);
    EXPECT_EQ(tu.data[main_class]["main"], helper::kind_t::method);
    EXPECT_EQ(tu.entry_point, tu.data[main_class]["main"].label);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
