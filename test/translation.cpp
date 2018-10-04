#include "error.h"
#include "parser.h"
#include "translate.h"
#include "gtest/gtest.h"
#include <fstream>

class translatorTest : public ::testing::Test {
  protected:
    IRBuilderTest() : tree() { no_err = tu.check(); }

    IR::Tree        tree;
    TranslationUnit tu("input/sample.miniJava");
    bool            no_err = true;
};

TEST_F(translatorTest, tuActuallyParses) { EXPECT_TRUE(no_err); }

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
