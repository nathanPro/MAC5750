#include "translate.h"

namespace IR
{
int translate(Tree& tree, AST::Exp const& exp)
{
    struct Visitor {
        Tree& t;
        int   operator()(AST::sumExp const& sum)
        {
            IRBuilder builder(t);
            builder << IRTag::BINOP << IR::BinopId::PLUS
                    << Grammar::visit(*this, sum.lhs)
                    << Grammar::visit(*this, sum.rhs);
            return builder.build();
        }
        int operator()(AST::integerExp const& exp)
        {
            IRBuilder builder(t);
            builder << IRTag::CONST << exp.value;
            return builder.build();
        }
        int operator()(AST::andExp const&) { return -1; }
        int operator()(AST::lessExp const&) { return -1; }
        int operator()(AST::minusExp const&) { return -1; }
        int operator()(AST::prodExp const&) { return -1; }
        int operator()(AST::indexingExp const&) { return -1; }
        int operator()(AST::lengthExp const&) { return -1; }
        int operator()(AST::methodCallExp const&) { return -1; }
        int operator()(AST::trueExp const&) { return -1; }
        int operator()(AST::falseExp const&) { return -1; }
        int operator()(AST::thisExp const&) { return -1; }
        int operator()(AST::identifierExp const&) { return -1; }
        int operator()(AST::newArrayExp const&) { return -1; }
        int operator()(AST::newObjectExp const&) { return -1; }
        int operator()(AST::bangExp const&) { return -1; }
        int operator()(AST::parenExp const&) { return -1; }
        int operator()(AST::ExpListRule const&) { return -1; }
    };
    return Grammar::visit(Visitor{tree}, exp);
}
} // namespace IR
