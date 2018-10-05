#include "IR.h"
#include "IRBuilder.h"
#include "gtest/gtest.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push

class catamorphismTest : public ::testing::Test {
  protected:
    catamorphismTest() {
        root = [&] {
            IRBuilder builder(tree);
            builder << IR::IRTag::CONST << 0;
            return builder.build();
        }();
        for (int i = 0; i < 10; i++) {
            IRBuilder sum(tree);
            IRBuilder lhs(tree);
            lhs << IR::IRTag::CONST << i;
            sum << IR::IRTag::BINOP << IR::BinopId::PLUS << root
                << lhs.build();
            root = sum.build();
        }
    }

    IR::Tree tree;
    int      root;
};

template <typename C> struct Counter {
    int operator()(const IR::Binop& binop) {
        return 1 + fmap(binop.lhs) + fmap(binop.rhs);
    }
    template <typename T> int operator()(const T& t) { return 1; }

    Counter(C&& __fmap) : fmap(__fmap) {}
    C fmap;
};

TEST_F(catamorphismTest, countNodesCata) {
    IR::Catamorphism<Counter, int> F(tree);
    EXPECT_EQ(F(root), 21);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
