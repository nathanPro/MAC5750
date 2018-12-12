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

    EXPECT_EQ(data.count(main_class), 1);
    EXPECT_EQ(data.count(help_class), 1);
    EXPECT_EQ(data.count(fake_class), 0);
}

TEST_F(HelperTest, MetaDataDistinguishesMethodFromVariable)
{
    EXPECT_EQ(data["Fac"]["t1"], helper::kind_t::var);
    EXPECT_EQ(data["Fac"]["Compute"], helper::kind_t::notfound);
    EXPECT_EQ(data["Fac"]["ComputeFac"], helper::kind_t::method);
}

TEST_F(HelperTest, MetaDataRecordLayout)
{
    EXPECT_EQ(data["Fac"].layout["t1"], 0);
    EXPECT_EQ(data["Fac"].layout["t2"], 4);
    EXPECT_EQ(data["Factorial"].size(), 0);
    EXPECT_EQ(data["Fac"].size(), 8);
    EXPECT_EQ(data["Child"].size(), data["Fac"].size());
}

TEST_F(HelperTest, MetaDataHandlesInheritance)
{
    EXPECT_EQ(data["Child"]["ComputeFac"], helper::kind_t::method);
}

TEST_F(HelperTest, MetaDataWorksOutOfOrder)
{
    TranslationUnit unordered("../input/unordered_classes.miniJava");
    EXPECT_TRUE(unordered.check());
    EXPECT_NO_THROW(helper::meta_data(unordered.syntax_tree));
}

TEST_F(HelperTest, MetaDataDetectsCyclicDepdendencies)
{
    TranslationUnit cycle("../input/cyclic_classes.miniJava");
    EXPECT_TRUE(cycle.check());
    EXPECT_THROW(helper::meta_data(cycle.syntax_tree),
                 helper::cyclic_classes);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
