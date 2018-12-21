#include "translate.h"

namespace IR
{
int translate(Tree& tree, AST::Exp const& exp)
{
    struct Visitor {
        Tree& t;

        int binop(IR::BinopId                                id,
                  AST::__detail::BinaryRule<AST::Exp> const& exp)
        {
            IRBuilder ans(t);
            ans << IRTag::BINOP << id
                << Grammar::visit(*this, exp.lhs)
                << Grammar::visit(*this, exp.rhs);
            return ans.build();
        }

        int operator()(AST::andExp const& exp)
        {
            return binop(IR::BinopId::AND, exp);
        }

        int operator()(AST::sumExp const& exp)
        {
            return binop(IR::BinopId::PLUS, exp);
        }

        int operator()(AST::minusExp const& exp)
        {
            return binop(IR::BinopId::MINUS, exp);
        }

        int operator()(AST::prodExp const& exp)
        {
            return binop(IR::BinopId::MUL, exp);
        }

        int operator()(AST::integerExp const& exp)
        {
            IRBuilder builder(t);
            builder << IRTag::CONST << exp.value;
            return builder.build();
        }
        int operator()(AST::parenExp const& exp)
        {
            return Grammar::visit(*this, exp.inner);
        }

        int operator()(AST::lessExp const&) { return -1; }
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
        int operator()(AST::ExpListRule const&) { return -1; }
    };
    return Grammar::visit(Visitor{tree}, exp);
}

int translate(Tree& tree, AST::Stm const& stm)
{
    struct Visitor {
        Tree& t;

        int operator()(AST::blockStm const&) { return -1; }
        int operator()(AST::ifStm const&) { return -1; }
        int operator()(AST::whileStm const&) { return -1; }
        int operator()(AST::printStm const& stm)
        {
            IRBuilder builder(t);

            builder << IR::IRTag::CALL << 0;
            builder << t.keep_explist(Explist{translate(t, stm.exp)});
            return builder.build();
        }
        int operator()(AST::assignStm const&) { return -1; }
        int operator()(AST::indexAssignStm const&) { return -1; }
    };
    return Grammar::visit(Visitor{tree}, stm);
}

} // namespace IR
