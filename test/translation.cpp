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
}
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
