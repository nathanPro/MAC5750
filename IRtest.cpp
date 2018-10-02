#include "IRBuilder.h"

bool create_const_exp() {
    IR tree;
    IRBuilder builder(tree);
    builder << IR::ExpType::CONST << 42;
    auto ref = builder.Exp();

    try {
        auto exp = Exp(tree, ref);
    } catch (IRBadAccess) {
        return false;
    }
    return true;
}
