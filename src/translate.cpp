#include "translate.h"

namespace IR
{
int translate(Tree& tree, AST::Exp const& exp)
{
    auto vis = Util::type_switch{

        [&](AST::sumExp const& sum) {
            IRBuilder builder(tree);
            builder << IRTag::BINOP << IR::BinopId::PLUS
                    << translate(tree, sum.lhs)
                    << translate(tree, sum.rhs);
            return builder.build();
        },

        [&](AST::integerExp const& exp) {
            IRBuilder builder(tree);
            builder << IRTag::CONST << exp.value;
            return builder.build();
        },

        [&](auto&&) { return -1; }

    };
    return Grammar::visit(vis, exp);
}
} // namespace IR
