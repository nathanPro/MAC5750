#include "error.h"
#include "parser.h"
#include "translate.h"
#include "gtest/gtest.h"
#include <sstream>

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push

TEST(parsingTest, sample)
{
    TranslationUnit tu("../input/sample.miniJava");
    EXPECT_TRUE(tu.check());
}

TEST(parsingTest, unordered_classes)
{
    TranslationUnit tu("../input/unordered_classes.miniJava");
    EXPECT_TRUE(tu.check());
}

TEST(parsingTest, cyclic_classes)
{
    TranslationUnit tu("../input/cyclic_classes.miniJava");
    EXPECT_TRUE(tu.check());
}

TEST(parsingTest, stringParsing)
{
    auto     stream = std::stringstream("3 + 4\n");
    auto     parser = Parser(&stream);
    auto     root   = parser.Exp();
    Reporter rep(std::cout, parser.errors);
    Grammar::visit(rep, root);
    EXPECT_TRUE(rep);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
