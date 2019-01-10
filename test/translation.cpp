#include "error.h"
#include "helper.h"
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

TEST(translatorTest, translateTrueAndFalse)
{
    IR::Tree    t[2];
    const char* in[2] = {"false\n", "true\n"};

    for (int i = 0; i < 2; i++) {
        auto stream = std::stringstream(in[i]);
        auto ir     = IR::translate(t[i], Parser(&stream).Exp());

        ASSERT_NE(ir, -1);
        EXPECT_EQ(t[i].get_type(ir), IR::IRTag::CONST);
        EXPECT_EQ(t[i].get_const(ir).value, i);
    }
}

TEST(translatorTest, translatelessExp)
{
    IR::Tree tree;
    auto     stream = std::stringstream("3 < 4\n");
    auto     ir     = IR::translate(tree, Parser(&stream).Exp());

    ASSERT_NE(ir, -1);
    EXPECT_EQ(tree.get_type(ir), IR::IRTag::MOVE);
    auto m = tree.get_move(ir);

    ASSERT_NE(m.dst, -1);
    EXPECT_EQ(tree.get_type(m.dst), IR::IRTag::TEMP);

    ASSERT_NE(m.src, -1);
    EXPECT_EQ(tree.get_type(m.src), IR::IRTag::CMP);
    auto cmp = tree.get_cmp(m.src);
    EXPECT_EQ(tree.get_type(cmp.lhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_type(cmp.rhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_const(cmp.lhs).value, 3);
    EXPECT_EQ(tree.get_const(cmp.rhs).value, 4);
}

TEST(translatorTest, translateBangExp)
{
    IR::Tree tree;
    auto     stream = std::stringstream("!(3 < 4)\n");
    auto     ir     = IR::translate(tree, Parser(&stream).Exp());

    ASSERT_NE(ir, -1);
    EXPECT_EQ(tree.get_type(ir), IR::IRTag::BINOP);
    auto [op, lhs, rhs] = tree.get_binop(ir);

    EXPECT_EQ(op, IR::BinopId::XOR);

    ASSERT_NE(lhs, -1);
    EXPECT_EQ(tree.get_type(lhs), IR::IRTag::TEMP);

    ASSERT_NE(rhs, -1);
    EXPECT_EQ(tree.get_type(rhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_const(rhs).value, 1);

    ASSERT_EQ(tree.stm_seq.size(), 1);
    EXPECT_EQ(tree.get_type(tree.stm_seq.back()), IR::IRTag::MOVE);
    auto mv = tree.get_move(tree.stm_seq.back());
    EXPECT_EQ(mv.dst, lhs);
}

TEST(translatorTest, translatePrint)
{
    IR::Tree tree;
    auto stream = std::stringstream("System.out.println(21 * 2);\n");
    auto ir     = IR::translate(tree, Parser(&stream).Stm());
    ASSERT_NE(ir, -1);
    EXPECT_EQ(tree.get_type(ir), IR::IRTag::EXP);

    ASSERT_EQ(tree.stm_seq.size(), 2);

    EXPECT_EQ(tree.get_type(tree.stm_seq[0]), IR::IRTag::MOVE);

    int id;
    {
        int tmp = tree.get_move(tree.stm_seq[0]).dst;
        EXPECT_EQ(tree.get_type(tmp), IR::IRTag::TEMP);
        id = tree.get_temp(tmp).id;
    }

    {
        int bnp = tree.get_move(tree.stm_seq[0]).src;
        EXPECT_EQ(tree.get_type(bnp), IR::IRTag::BINOP);
        auto [op, lhs, rhs] = tree.get_binop(bnp);
        EXPECT_EQ(op, IR::BinopId::MUL);
        EXPECT_EQ(tree.get_type(lhs), IR::IRTag::CONST);
        EXPECT_EQ(tree.get_const(lhs).value, 21);
        EXPECT_EQ(tree.get_type(rhs), IR::IRTag::CONST);
        EXPECT_EQ(tree.get_const(rhs).value, 2);
    }

    EXPECT_EQ(tree.get_type(tree.stm_seq[1]), IR::IRTag::EXP);
    EXPECT_EQ(tree.get_type(tree.get_exp(tree.stm_seq[1]).exp),
              IR::IRTag::CALL);

    {
        auto [fn, _es] =
            tree.get_call(tree.get_exp(tree.stm_seq[1]).exp);
        EXPECT_EQ(fn, 0);
        auto es = tree.get_explist(_es);
        EXPECT_EQ(es.size(), 1);
        EXPECT_EQ(tree.get_type(es[0]), IR::IRTag::TEMP);
        EXPECT_EQ(tree.get_temp(es[0]).id, id);
    }
}

TEST(translatorTest, translatesFullCalc)
{
    IR::Tree        tree;
    TranslationUnit tu("../input/calc.miniJava");
    EXPECT_TRUE(tu.check());
    translate(tree, tu.syntax_tree);
    EXPECT_EQ(tree.methods.size(), 1);
    EXPECT_EQ(tree.methods.begin()->first, std::string("main"));

    auto const& main_frag = tree.methods.begin()->second;
    EXPECT_EQ(main_frag.size(), 2);
}

TEST(translatorTest, translatesFullSample)
{
    IR::Tree        tree;
    TranslationUnit tu("../input/sample.miniJava");
    EXPECT_TRUE(tu.check());
    translate(tree, tu.syntax_tree);
    EXPECT_EQ(tree.methods.size(), 2);

    std::set<std::string> frags = {
        std::string("main"), helper::mangle("Fac", "ComputeFac")};

    for (auto it : tree.methods) {
        auto f = frags.find(it.first);
        EXPECT_NE(f, frags.end());
        frags.erase(f);
    }

    EXPECT_EQ(frags.size(), 0);
}

TEST(translatorTest, translatesIdentifierExp)
{
    IR::Tree        tree;
    TranslationUnit tu("../input/sample2.miniJava");
    EXPECT_TRUE(tu.check());
    translate(tree, tu.syntax_tree);
    EXPECT_EQ(tree.methods.size(), 2);

    auto const lbl = helper::mangle("Fac", "ComputeFac");
    ASSERT_NE(tree.methods.count(lbl), 0);

    auto const& frag = tree.methods[lbl];
    ASSERT_NE(frag.stms.size(), 0);

    auto ret_stm = frag.stms.back();
    EXPECT_EQ(tree.get_type(ret_stm), IR::IRTag::EXP);
    auto ret_ref = tree.get_exp(ret_stm).exp;
    EXPECT_EQ(tree.get_type(ret_ref), IR::IRTag::MEM);
    EXPECT_EQ(tree.get_type(tree.get_mem(ret_ref).exp),
              IR::IRTag::BINOP);

    auto [op, lhs, rhs] = tree.get_binop(tree.get_mem(ret_ref).exp);
    EXPECT_EQ(op, IR::BinopId::PLUS);
    EXPECT_EQ(tree.get_type(lhs), IR::IRTag::TEMP);
    EXPECT_EQ(lhs, frag.stack.sp);

    EXPECT_EQ(tree.get_type(rhs), IR::IRTag::CONST);
    EXPECT_EQ(tree.get_const(rhs).value, 0);
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
        case IR::IRTag::JMP:
            lhs = tree.get_jmp(i).target;
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
