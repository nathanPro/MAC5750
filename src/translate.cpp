#include "translate.h"

namespace IR
{

Translator::Translator(Tree& tree) : t(tree) {}
Translator::Translator(Tree& tree, helper::meta_data&& d)
    : t(tree), data(std::move(d))
{
}

int Translator::binop(BinopId                                    id,
                      AST::__detail::BinaryRule<AST::Exp> const& exp)
{
    IRBuilder ans(t);
    ans << IRTag::BINOP << id << Grammar::visit(*this, exp.lhs)
        << Grammar::visit(*this, exp.rhs);
    return ans.build();
}

int Translator::operator()(AST::andExp const& exp)
{
    return binop(IR::BinopId::AND, exp);
}

int Translator::operator()(AST::sumExp const& exp)
{
    return binop(IR::BinopId::PLUS, exp);
}

int Translator::operator()(AST::minusExp const& exp)
{
    return binop(IR::BinopId::MINUS, exp);
}

int Translator::operator()(AST::prodExp const& exp)
{
    return binop(IR::BinopId::MUL, exp);
}

int Translator::operator()(AST::integerExp const& exp)
{
    IRBuilder builder(t);
    builder << IRTag::CONST << exp.value;
    return builder.build();
}

int Translator::operator()(AST::trueExp const&)
{
    IRBuilder builder(t);
    builder << IRTag::CONST << 1;
    return builder.build();
}

int Translator::operator()(AST::falseExp const&)
{
    IRBuilder builder(t);
    builder << IRTag::CONST << 0;
    return builder.build();
}

int Translator::operator()(AST::parenExp const& exp)
{
    return Grammar::visit(*this, exp.inner);
}

int Translator::operator()(AST::lessExp const& exp)
{
    IRBuilder root(t);
    IRBuilder tmp(t);
    IRBuilder cmp(t);

    tmp << IRTag::TEMP;
    cmp << IRTag::CMP << Grammar::visit(*this, exp.lhs)
        << Grammar::visit(*this, exp.rhs);

    root << IRTag::MOVE << tmp.build() << cmp.build();
    return root.build();
}

int Translator::operator()(AST::bangExp const& exp)
{
    auto mv  = Grammar::visit(*this, exp.inner);
    auto tmp = t.get_move(mv).dst;

    IRBuilder root(t);
    IRBuilder cte(t);
    cte << IR::IRTag::CONST << 1;

    root << IR::IRTag::BINOP << IR::BinopId::XOR << tmp
         << cte.build();

    return root.build();
}

int Translator::operator()(AST::ExpListRule const&) { return -1; }
int Translator::operator()(AST::lengthExp const&) { return -1; }
int Translator::operator()(AST::methodCallExp const&) { return -1; }
int Translator::operator()(AST::thisExp const&) { return -1; }
int Translator::operator()(AST::identifierExp const&) { return -1; }
int Translator::operator()(AST::indexingExp const&) { return -1; }
int Translator::operator()(AST::newArrayExp const&) { return -1; }
int Translator::operator()(AST::newObjectExp const&) { return -1; }
int Translator::operator()(AST::blockStm const&) { return -1; }
int Translator::operator()(AST::ifStm const&) { return -1; }
int Translator::operator()(AST::whileStm const&) { return -1; }
int Translator::operator()(AST::printStm const& stm)
{
    IRBuilder exp(t);
    exp << IR::IRTag::EXP << [&] {
        IRBuilder builder(t);

        builder << IR::IRTag::CALL << 0;
        builder << t.keep_explist(Explist{
            store_in_temp(t, Grammar::visit(*this, stm.exp))});
        return builder.build();
    }();
    return exp.build();
}
int Translator::operator()(AST::assignStm const&) { return -1; }
int Translator::operator()(AST::indexAssignStm const&) { return -1; }

int Translator::operator()(AST::ClassDeclNoInheritance const&)
{
    return -1;
}
int Translator::operator()(AST::ClassDeclInheritance const&)
{
    return -1;
}
int Translator::operator()(AST::MainClassRule const& mc)
{
    current_class  = mc.name;
    current_method = "main";

    Grammar::visit(*this, mc.body);

    t.methods.insert({helper::mangle(current_class, current_method),
                      std::move(t.stm_seq)});
    t.stm_seq = {};
    return 0;
}
int Translator::operator()(AST::ProgramRule const& prog)
{
    Grammar::visit(*this, prog.main);
    for (auto const& c : prog.classes) Grammar::visit(*this, c);
    return 0;
}

int translate(Tree& t, AST::Exp const& exp)
{
    return Grammar::visit(Translator{t}, exp);
}

int translate(Tree& t, AST::Stm const& stm)
{
    return Grammar::visit(Translator{t}, stm);
}

void translate(Tree& t, AST::Program const& p)
{
    Grammar::visit(Translator{t, helper::meta_data(p)}, p);
}

} // namespace IR
