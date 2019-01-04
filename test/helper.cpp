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
    EXPECT_EQ(data["Fac"].variable["t1"], 0);
    EXPECT_EQ(data["Fac"].variable["t2"], 8);
    EXPECT_EQ(data["Factorial"].size(), 0);
    EXPECT_EQ(data["Fac"].size(), 16);
    EXPECT_EQ(data["Child"].size(), data["Fac"].size());
}

TEST_F(HelperTest, MetaDataHandlesInheritance)
{
    EXPECT_EQ(data["Child"]["ComputeFac"], helper::kind_t::method);
    EXPECT_EQ(data["Fac"].method("ComputeFac").layout["num_aux"],
              data["Child"].method("ComputeFac").layout["num_aux"]);
    EXPECT_EQ(data["Fac"].method("ComputeFac").layout["not_aux"],
              data["Child"].method("ComputeFac").layout["not_aux"]);
}

TEST_F(HelperTest, MetaDataKeepsNames)
{
    EXPECT_EQ(data["Child"].name, std::string("Child"));
    EXPECT_EQ(data["Child"].method("ComputeFac").name,
              std::string("ComputeFac"));
}

TEST_F(HelperTest, MetaDataWorksOutOfOrder)
{
    TranslationUnit unordered("../input/unordered_classes.miniJava");
    EXPECT_TRUE(unordered.check());
    EXPECT_NO_THROW(helper::meta_data(unordered.syntax_tree));
}

TEST_F(HelperTest, MetaDataHandlesDeeperInheritanceVariables)
{
    TranslationUnit unordered("../input/unordered_classes.miniJava");
    EXPECT_TRUE(unordered.check());
    data = helper::meta_data(unordered.syntax_tree);

    EXPECT_EQ(data["A"]["t1"], helper::kind_t::var);
    EXPECT_EQ(data["B"]["t1"], helper::kind_t::var);
    EXPECT_EQ(data["C"]["t1"], helper::kind_t::var);

    EXPECT_EQ(data["A"].variable["__base"], 0);
    EXPECT_EQ(data["A"].variable["t4"], 24);

    EXPECT_EQ(data["B"].variable["__base"], 0);
    EXPECT_EQ(data["B"].variable["t3"], 16);

    EXPECT_EQ(data["C"].variable["t1"], 0);
    EXPECT_EQ(data["C"].variable["t2"], 8);

    EXPECT_EQ(data["A"].method("calculate").position("t1"), 0);
    EXPECT_EQ(data["A"].method("calculate").position("t2"), 8);
    EXPECT_EQ(data["A"].method("calculate").position("t3"), 16);
    EXPECT_EQ(data["A"].method("calculate").position("t4"), 24);
}

TEST_F(HelperTest, MetaDataHandlesDeeperInheritanceMethods)
{
    TranslationUnit unordered("../input/unordered_classes.miniJava");
    EXPECT_TRUE(unordered.check());
    data = helper::meta_data(unordered.syntax_tree);

    EXPECT_EQ(data["A"]["calculate"], helper::kind_t::method);
    EXPECT_EQ(data["B"]["calculate"], helper::kind_t::method);
    EXPECT_EQ(data["C"]["calculate"], helper::kind_t::method);

    EXPECT_EQ(data["A"].method("calculate").layout["num_aux"],
              data["C"].method("calculate").layout["num_aux"]);
    EXPECT_EQ(data["A"].method("calculate").layout["not_aux"],
              data["C"].method("calculate").layout["not_aux"]);
}

TEST_F(HelperTest, MetaDataDetectsCyclicDepdendencies)
{
    TranslationUnit cycle("../input/cyclic_classes.miniJava");
    EXPECT_TRUE(cycle.check());
    EXPECT_THROW(helper::meta_data(cycle.syntax_tree),
                 helper::cyclic_classes);
}

TEST_F(HelperTest, MetaDataStoresStackLayout)
{
    EXPECT_EQ(data["Fac"].method("ComputeFac").layout["num_aux"], 0);
    EXPECT_EQ(data["Fac"].method("ComputeFac").layout["not_aux"], 8);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
