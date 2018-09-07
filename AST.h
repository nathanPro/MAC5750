#include "grammar.h"
#include "string"
#include <memory>
#include <variant>
#include <vector>
#define UNREACHABLE() (__builtin_unreachable())

namespace AST {
struct Program;
struct MainClass;
struct ClassDecl;
struct VarDecl;
struct MethodDecl;
struct FormalList;
struct Type;
struct Stm;
struct Exp;
struct ExpList;

namespace __detail {
using pProgram    = std::unique_ptr<Program>;
using pClassDecl  = std::unique_ptr<ClassDecl>;
using pExp        = std::unique_ptr<Exp>;
using pExpList    = std::unique_ptr<ExpList>;
using pFormalList = std::unique_ptr<FormalList>;
using pMainClass  = std::unique_ptr<MainClass>;
using pMethodDecl = std::unique_ptr<MethodDecl>;
using pStm        = std::unique_ptr<Stm>;
using pType       = std::unique_ptr<Type>;
using pVarDecl    = std::unique_ptr<VarDecl>;
} // namespace __detail

struct ProgramRule {
    __detail::pMainClass main = nullptr;
    std::vector<__detail::pClassDecl> classes;
};

struct Program : Grammar::Nonterminal<std::variant<ProgramRule>> {
    using Grammar::Nonterminal<Program::variant_t>::Nonterminal;
};

struct MainClassRule {
    std::string name;
    std::string argument;
    __detail::pStm body = nullptr;
};

struct MainClass : Grammar::Nonterminal<std::variant<MainClassRule>> {
    using Grammar::Nonterminal<MainClass::variant_t>::Nonterminal;
};

struct ClassDeclNoInheritance {
    std::string name;
    std::vector<__detail::pVarDecl> variables;
    std::vector<__detail::pMethodDecl> methods;
};

struct ClassDeclInheritance {
    std::string name;
    std::string superclass;
    std::vector<__detail::pVarDecl> variables;
    std::vector<__detail::pMethodDecl> methods;
};

// clang-format off
struct ClassDecl : Grammar::Nonterminal<std::variant<
                   ClassDeclNoInheritance,
                   ClassDeclInheritance>>
{
    using Grammar::Nonterminal<ClassDecl::variant_t>::Nonterminal;
};
// clang-format on

struct VarDeclRule {
    __detail::pType type = nullptr;
    std::string name;
};

struct VarDecl : Grammar::Nonterminal<std::variant<VarDeclRule>> {
    using Grammar::Nonterminal<VarDecl::variant_t>::Nonterminal;
};

struct MethodDeclRule {
    __detail::pType type = nullptr;
    std::string name;
    __detail::pFormalList arguments;
    std::vector<__detail::pVarDecl> variables;
    std::vector<__detail::pStm> body;
    __detail::pExp return_exp = nullptr;
};

struct MethodDecl
    : Grammar::Nonterminal<std::variant<MethodDeclRule>> {
    using Grammar::Nonterminal<MethodDecl::variant_t>::Nonterminal;
};

struct FormalDecl {
    __detail::pType type = nullptr;
    std::string name;
};

struct FormalListRule {
    std::vector<FormalDecl> decls;
};

struct FormalList
    : Grammar::Nonterminal<std::variant<FormalListRule>> {
    using Grammar::Nonterminal<FormalList::variant_t>::Nonterminal;
};

struct integerArrayType {};
struct booleanType {};
struct integerType {};
struct classType {
    std::string name;
};

// clang-format off
struct Type : Grammar::Nonterminal<std::variant<
    integerArrayType,
    booleanType,
    integerType,
    classType
>>
{
    using Grammar::Nonterminal<Type::variant_t>::Nonterminal;
};
// clang-format on

struct blockStm {
    std::vector<__detail::pStm> statements;
};
struct ifStm {
    __detail::pExp condition   = nullptr;
    __detail::pStm if_clause   = nullptr;
    __detail::pStm else_clause = nullptr;
};
struct whileStm {
    __detail::pExp condition = nullptr;
    __detail::pStm body;
};
struct printStm {
    __detail::pExp exp = nullptr;
};
struct assignStm {
    std::string name;
    __detail::pExp value = nullptr;
};
struct indexAssignStm {
    std::string array;
    __detail::pExp index = nullptr;
    __detail::pExp value = nullptr;
};

// clang-format off
struct Stm : Grammar::Nonterminal<std::variant<
             blockStm,
             ifStm,
             whileStm,
             printStm,
             assignStm,
             indexAssignStm>>
{
    using Grammar::Nonterminal<Stm::variant_t>::Nonterminal;
};
// clang-format on

using andExp = Grammar::BinaryRule<struct andExpTag, __detail::pExp>;
using lessExp =
    Grammar::BinaryRule<struct lessExpTag, __detail::pExp>;
using sumExp = Grammar::BinaryRule<struct sumExpTag, __detail::pExp>;
using minusExp =
    Grammar::BinaryRule<struct minusExpTag, __detail::pExp>;
using prodExp =
    Grammar::BinaryRule<struct prodExpTag, __detail::pExp>;
struct indexingExp {
    __detail::pExp array = nullptr;
    __detail::pExp index = nullptr;
};
using lengthExp =
    Grammar::UnaryRule<struct lengthExpTag, __detail::pExp>;
struct methodCallExp {
    __detail::pExp object = nullptr;
    std::string name;
    __detail::pExpList arguments = nullptr;
};
struct integerExp {
    int32_t value;
};
struct trueExp {};
struct falseExp {};
struct thisExp {};
struct identifierExp {
    std::string name;
};
using newArrayExp =
    Grammar::UnaryRule<struct arrayExpTag, __detail::pExp>;
struct newObjectExp {
    std::string name;
};
using bangExp = Grammar::UnaryRule<struct bangExpTag, __detail::pExp>;
using parenExp =
    Grammar::UnaryRule<struct parenExpTag, __detail::pExp>;

// clang-format off
struct Exp : Grammar::Nonterminal<std::variant<
    andExp,
    lessExp,
    sumExp,
    minusExp,
    prodExp,
    indexingExp,
    lengthExp,
    methodCallExp,
    integerExp,
    trueExp,
    falseExp,
    thisExp,
    identifierExp,
    newArrayExp,
    newObjectExp,
    bangExp,
    parenExp>>
{
    using Grammar::Nonterminal<Exp::variant_t>::Nonterminal;
};
// clang-format on

struct ExpListRule {
    std::vector<__detail::pExp> exps;
};

struct ExpList : Grammar::Nonterminal<std::variant<ExpListRule>> {
    using Grammar::Nonterminal<ExpList::variant_t>::Nonterminal;
};

} // namespace AST
