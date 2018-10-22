#include "error.h"
#include "parser.h"
#include "translate.h"
#include "gtest/gtest.h"
#include <sstream>

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push

class translatorTest : public ::testing::Test {
  protected:
    translatorTest() : tu("../input/sample.miniJava") {
        no_err = tu.check();
    }

    IR::Tree        tree;
    TranslationUnit tu;
    bool            no_err = true;
};

TEST_F(translatorTest, tuActuallyParses) { EXPECT_TRUE(no_err); }

TEST_F(translatorTest, tuRootIsProgram) {
    bool is_prog           = false;
    bool visited_something = false;
    Grammar::visit(
        Util::type_switch{
            [&](const AST::ProgramRule& prog) {
                is_prog           = true;
                visited_something = true;
            },
            [&](const auto& n) { visited_something = true; }},
        tu.syntax_tree);
    EXPECT_TRUE(is_prog);
    EXPECT_TRUE(visited_something);
}

TEST_F(translatorTest, translateSumExp) {
    auto root              = Parser(std::stringstream("3 + 4")).Exp();
    bool is_sumExp         = false;
    bool visited_something = false;

    Grammar::visit(Util::type_switch{[&](const AST::sumExp& exp) {
                                         is_sumExp         = true;
                                         visited_something = true;
                                     },
                                     [&](const auto& n) {
                                         visited_something = true;
                                     }},
                   root);
    EXPECT_TRUE(is_sumExp);
    EXPECT_TRUE(visited_something);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
