#ifndef AST_H
#define AST_H

#include "grammar.h"
#include "string"
#include "util.h"
#include <memory>
#include <variant>
#include <vector>
#define UNREACHABLE() (__builtin_unreachable())

namespace AST {
template <typename T> using ptr = std::unique_ptr<T>;
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
using pProgram    = ptr<Program>;
using pClassDecl  = ptr<ClassDecl>;
using pExp        = ptr<Exp>;
using pExpList    = ptr<ExpList>;
using pFormalList = ptr<FormalList>;
using pMainClass  = ptr<MainClass>;
using pMethodDecl = ptr<MethodDecl>;
using pStm        = ptr<Stm>;
using pType       = ptr<Type>;
using pVarDecl    = ptr<VarDecl>;
} // namespace __detail

using Node = Entity<struct NodeTag>;

struct ProgramRule {
    Node id;
    __detail::pMainClass main = nullptr;
    std::vector<__detail::pClassDecl> classes;
};

struct Program : Grammar::Nonterminal<std::variant<ProgramRule>> {
    using Grammar::Nonterminal<Program::variant_t>::Nonterminal;
};

struct MainClassRule {
    Node id;
    std::string name;
    std::string argument;
    __detail::pStm body = nullptr;
};

struct MainClass : Grammar::Nonterminal<std::variant<MainClassRule>> {
    using Grammar::Nonterminal<MainClass::variant_t>::Nonterminal;
};

struct ClassDeclNoInheritance {
    Node id;
    std::string name;
    std::vector<__detail::pVarDecl> variables;
    std::vector<__detail::pMethodDecl> methods;
};

struct ClassDeclInheritance {
    Node id;
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
    Node id;
    __detail::pType type = nullptr;
    std::string name;
};

struct VarDecl : Grammar::Nonterminal<std::variant<VarDeclRule>> {
    using Grammar::Nonterminal<VarDecl::variant_t>::Nonterminal;
};

struct MethodDeclRule {
    Node id;
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
    Node id;
    __detail::pType type = nullptr;
    std::string name;
};

struct FormalListRule {
    Node id;
    std::vector<FormalDecl> decls;
};

struct FormalList
    : Grammar::Nonterminal<std::variant<FormalListRule>> {
    using Grammar::Nonterminal<FormalList::variant_t>::Nonterminal;
};

struct integerArrayType {
    Node id;
};
struct booleanType {
    Node id;
};
struct integerType {
    Node id;
};
struct classType {
    Node id;
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
    Node id;
    std::vector<__detail::pStm> statements;
};
struct ifStm {
    Node id;
    __detail::pExp condition   = nullptr;
    __detail::pStm if_clause   = nullptr;
    __detail::pStm else_clause = nullptr;
};
struct whileStm {
    Node id;
    __detail::pExp condition = nullptr;
    __detail::pStm body;
};
struct printStm {
    Node id;
    __detail::pExp exp = nullptr;
};
struct assignStm {
    Node id;
    std::string name;
    __detail::pExp value = nullptr;
};
struct indexAssignStm {
    Node id;
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

using andExp =
    Grammar::BinaryRule<struct andExpTag, Node, __detail::pExp>;
using lessExp =
    Grammar::BinaryRule<struct lessExpTag, Node, __detail::pExp>;
using sumExp =
    Grammar::BinaryRule<struct sumExpTag, Node, __detail::pExp>;
using minusExp =
    Grammar::BinaryRule<struct minusExpTag, Node, __detail::pExp>;
using prodExp =
    Grammar::BinaryRule<struct prodExpTag, Node, __detail::pExp>;
struct indexingExp {
    Node id;
    __detail::pExp array = nullptr;
    __detail::pExp index = nullptr;
};
using lengthExp =
    Grammar::UnaryRule<struct lengthExpTag, Node, __detail::pExp>;
struct methodCallExp {
    Node id;
    __detail::pExp object = nullptr;
    std::string name;
    __detail::pExpList arguments = nullptr;
};
struct integerExp {
    Node id;
    int32_t value;
};
struct trueExp {
    Node id;
};
struct falseExp {
    Node id;
};
struct thisExp {
    Node id;
};
struct identifierExp {
    Node id;
    std::string name;
};
using newArrayExp =
    Grammar::UnaryRule<struct arrayExpTag, Node, __detail::pExp>;
struct newObjectExp {
    Node id;
    std::string name;
};
using bangExp =
    Grammar::UnaryRule<struct bangExpTag, Node, __detail::pExp>;
using parenExp =
    Grammar::UnaryRule<struct parenExpTag, Node, __detail::pExp>;

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
    Node id;
    std::vector<__detail::pExp> exps;
};

struct ExpList : Grammar::Nonterminal<std::variant<ExpListRule>> {
    using Grammar::Nonterminal<ExpList::variant_t>::Nonterminal;
};

} // namespace AST
#endif
