#include "error.h"
#include "parser.h"
#include "translate.h"
#include "gtest/gtest.h"
#include <sstream>
#include <tuple>

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

TEST(translatorTest, translatePrint)
{
    IR::Tree tree;
    auto stream = std::stringstream("System.out.println(21 * 2);\n");
    auto ir     = IR::translate(tree, Parser(&stream).Stm());
    ASSERT_NE(ir, -1);
    EXPECT_EQ(tree.get_type(ir), IR::IRTag::CALL);
    auto [f, _es] = tree.get_call(ir);
    EXPECT_EQ(f, 0);

    auto es = tree.get_explist(_es);
    EXPECT_EQ(es.size(), static_cast<size_t>(1));
    EXPECT_EQ(tree.get_type(es[0]), IR::IRTag::BINOP);
    EXPECT_EQ(tree.get_binop(es[0]).op, IR::BinopId::MUL);
}

TEST(translatorTest, IRPostOrdering)
{
    IR::Tree tree;
    auto stream = std::stringstream("System.out.println(21 * 2);\n");
    auto ir     = IR::translate(tree, Parser(&stream).Stm());
    ASSERT_NE(ir, -1);

    std::vector<int> st;
    st.push_back(ir);
    while (!st.empty()) {
        int i = st.back();
        st.pop_back();

        int lhs = -1, rhs = -1;
        switch (tree.get_type(i)) {
        case IR::IRTag::BINOP:
            lhs = tree.get_binop(i).lhs;
            rhs = tree.get_binop(i).rhs;
            break;
        case IR::IRTag::MEM:
            lhs = tree.get_mem(i).exp;
            break;
        case IR::IRTag::MOVE:
            lhs = tree.get_move(i).dst;
            rhs = tree.get_move(i).src;
            break;
        case IR::IRTag::EXP:
            lhs = tree.get_exp(i).exp;
            break;
        case IR::IRTag::JUMP:
            lhs = tree.get_jump(i).target;
            break;
        case IR::IRTag::CALL:
            for (auto j : tree.get_explist(tree.get_call(i).explist))
                st.push_back(j);
            break;
        case IR::IRTag::NAME:
        case IR::IRTag::TEMP:
        case IR::IRTag::LABEL:
        case IR::IRTag::CONST:
        case IR::IRTag::CMP:
        case IR::IRTag::CJMP:
            break;
        }
        if (lhs != -1) {
            ASSERT_LT(lhs, i);
            st.push_back(lhs);
        }
        if (rhs != -1) {
            ASSERT_LT(rhs, i);
            st.push_back(rhs);
        }
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
