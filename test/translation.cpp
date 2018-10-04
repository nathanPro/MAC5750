#include "error.h"
#include "parser.h"
#include "translate.h"
#include "gtest/gtest.h"
#include <sstream>

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

TEST_F(translatorTest, translateSumExp) {
    std::stringstream exp_stream("3 + 4");
    auto              ctx  = ParserContext(exp_stream);
    auto              root = AST::translate(tree, Parser::Exp(ctx));
    EXPECT_EQ(tree.get_type(root),
              static_cast<int>(IR::ExpId::BINOP));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
