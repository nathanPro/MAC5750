#include "helper.h"
#include "error.h"
#include "parser.h"
#include "translate.h"
#include "gtest/gtest.h"

class HelperTest : public ::testing::Test
{
  protected:
    TranslationUnit   tu;
    bool              no_err;
    helper::meta_data data;

    HelperTest()
        : tu("../input/sample.miniJava"), no_err(tu.check()),
          data(tu.syntax_tree)
    {
    }
};

TEST_F(HelperTest, traverseASTbuildsMetaData) { EXPECT_TRUE(no_err); }

TEST_F(HelperTest, MetaDataRecordClasses)
{
    std::string const main_class("Factorial");
    std::string const help_class("Fac");
    std::string const fake_class("Fake");

    EXPECT_EQ(data.classes.count(main_class), 1);
    EXPECT_EQ(data.classes.count(help_class), 1);
    EXPECT_EQ(data.classes.count(fake_class), 0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
