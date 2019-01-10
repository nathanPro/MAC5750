#ifndef BCC_TRANSLATE
#define BCC_TRANSLATE

#include "AST.h"
#include "IR.h"
#include "IRBuilder.h"
#include "helper.h"

namespace IR
{
struct fragmentGuard;
class Translator
{
    Tree&             t;
    helper::meta_data data;
    std::string       current_class;
    std::string       current_method;
    activation_record frame;

    int binop(BinopId, AST::__detail::BinaryRule<AST::Exp> const&);

  public:
    Translator(Tree&);
    Translator(Tree&, helper::meta_data&&);

    int operator()(AST::andExp const& exp);
    int operator()(AST::sumExp const& exp);
    int operator()(AST::minusExp const& exp);
    int operator()(AST::prodExp const& exp);
    int operator()(AST::integerExp const& exp);
    int operator()(AST::trueExp const&);
    int operator()(AST::falseExp const&);
    int operator()(AST::parenExp const& exp);
    int operator()(AST::lessExp const& exp);
    int operator()(AST::bangExp const& exp);
    int operator()(AST::ExpListRule const&);
    int operator()(AST::lengthExp const&);
    int operator()(AST::methodCallExp const&);
    int operator()(AST::thisExp const&);
    int operator()(AST::identifierExp const&);
    int operator()(AST::indexingExp const&);
    int operator()(AST::newArrayExp const&);
    int operator()(AST::newObjectExp const&);
    int operator()(AST::blockStm const&);
    int operator()(AST::ifStm const&);
    int operator()(AST::whileStm const&);
    int operator()(AST::printStm const& stm);
    int operator()(AST::assignStm const&);
    int operator()(AST::indexAssignStm const&);

    int operator()(AST::ProgramRule const&);
    int operator()(AST::MainClassRule const&);
    int operator()(AST::ClassDeclNoInheritance const&);
    int operator()(AST::ClassDeclInheritance const&);
    int operator()(AST::MethodDeclRule const&);
};

struct fragmentGuard {
    Tree&             t;
    std::string       label;
    activation_record rec;

    fragmentGuard(Tree&, std::string, activation_record);
    ~fragmentGuard();
};

int  translate(Tree&, AST::Exp const&);
int  translate(Tree&, AST::Stm const&);
void translate(Tree&, AST::Program const&);
} // namespace IR

#endif
