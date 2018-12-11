#ifndef AST_H
#define AST_H

#include "grammar.h"
#include "lexer.h"
#include "util.h"
#include <algorithm>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#define UNREACHABLE() (__builtin_unreachable())

namespace AST
{
class Builder;
namespace __detail
{

struct TagRule : Grammar::Indexable {
    TagRule(Builder&& data);
};

template <typename nt_t> struct BinaryRule : Grammar::Indexable {
    nt_t lhs;
    nt_t rhs;
    BinaryRule(Builder&&);
};

template <typename nt_t> struct UnaryRule : Grammar::Indexable {
    nt_t inner;
    UnaryRule(Builder&&);
};

template <typename T> struct ValueWrapper : Grammar::Indexable {
    T value;
    ValueWrapper(Builder&&);
};
} // namespace __detail

struct andExp;
struct lessExp;
struct sumExp;
struct minusExp;
struct prodExp;
struct indexingExp;
struct lengthExp;
struct methodCallExp;
struct integerExp;
struct trueExp;
struct falseExp;
struct thisExp;
struct identifierExp;
struct newArrayExp;
struct newObjectExp;
struct bangExp;
struct parenExp;

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

struct ExpListRule : Grammar::Indexable {
    std::vector<Exp> exps;

    ExpListRule(Builder&&);
};

struct ExpList : Grammar::Nonterminal<std::variant<ExpListRule>> {
    using Grammar::Nonterminal<ExpList::variant_t>::Nonterminal;
};

struct andExp : __detail::BinaryRule<Exp> {
    using __detail::BinaryRule<Exp>::BinaryRule;
};
struct lessExp : __detail::BinaryRule<Exp> {
    using __detail::BinaryRule<Exp>::BinaryRule;
};
struct sumExp : __detail::BinaryRule<Exp> {
    using __detail::BinaryRule<Exp>::BinaryRule;
};
struct minusExp : __detail::BinaryRule<Exp> {
    using __detail::BinaryRule<Exp>::BinaryRule;
};
struct prodExp : __detail::BinaryRule<Exp> {
    using __detail::BinaryRule<Exp>::BinaryRule;
};
struct indexingExp : __detail::BinaryRule<Exp> {
    using __detail::BinaryRule<Exp>::BinaryRule;
};
struct lengthExp : __detail::UnaryRule<Exp> {
    using __detail::UnaryRule<Exp>::UnaryRule;
};

struct methodCallExp : Grammar::Indexable {
    Exp         object;
    std::string name;
    ExpList     arguments;

    methodCallExp(Builder&&);
};

struct integerExp : __detail::ValueWrapper<int32_t> {
    using __detail::ValueWrapper<int32_t>::ValueWrapper;
};
struct identifierExp : __detail::ValueWrapper<std::string> {
    using __detail::ValueWrapper<std::string>::ValueWrapper;
};
struct newObjectExp : __detail::ValueWrapper<std::string> {
    using __detail::ValueWrapper<std::string>::ValueWrapper;
};

struct trueExp : __detail::TagRule {
    using __detail::TagRule::TagRule;
};
struct falseExp : __detail::TagRule {
    using __detail::TagRule::TagRule;
};
struct thisExp : __detail::TagRule {
    using __detail::TagRule::TagRule;
};

struct newArrayExp : __detail::UnaryRule<Exp> {
    using __detail::UnaryRule<Exp>::UnaryRule;
};

struct bangExp : __detail::UnaryRule<Exp> {
    using __detail::UnaryRule<Exp>::UnaryRule;
};
struct parenExp : __detail::UnaryRule<Exp> {
    using __detail::UnaryRule<Exp>::UnaryRule;
};

struct blockStm;
struct ifStm;
struct whileStm;
struct printStm;
struct assignStm;
struct indexAssignStm;

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

struct blockStm : Grammar::Indexable {
    std::vector<Stm> statements;

    blockStm(Builder&&);
};

struct ifStm : Grammar::Indexable {
    Exp condition;
    Stm if_clause;
    Stm else_clause;

    ifStm(Builder&&);
};

struct whileStm : Grammar::Indexable {
    Exp condition;
    Stm body;

    whileStm(Builder&&);
};

struct printStm : Grammar::Indexable {
    Exp exp;

    printStm(Builder&&);
};

struct assignStm : Grammar::Indexable {
    std::string name;
    Exp         value;

    assignStm(Builder&&);
};

struct indexAssignStm : Grammar::Indexable {
    std::string array;
    Exp         index;
    Exp         value;

    indexAssignStm(Builder&&);
};

struct integerArrayType : __detail::TagRule {
    using __detail::TagRule::TagRule;
};
struct booleanType : __detail::TagRule {
    using __detail::TagRule::TagRule;
};
struct integerType : __detail::TagRule {
    using __detail::TagRule::TagRule;
};
struct classType : __detail::ValueWrapper<std::string> {
    using __detail::ValueWrapper<std::string>::ValueWrapper;
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

struct FormalDecl {
    Type        type;
    std::string name;
};

struct FormalListRule : Grammar::Indexable {
    std::vector<FormalDecl> decls;

    FormalListRule(Builder&&);
};

struct FormalList
    : Grammar::Nonterminal<std::variant<FormalListRule>> {
    using Grammar::Nonterminal<FormalList::variant_t>::Nonterminal;
};

struct VarDeclRule : Grammar::Indexable {
    Type        type;
    std::string name;

    VarDeclRule(Builder&&);
};

struct VarDecl : Grammar::Nonterminal<std::variant<VarDeclRule>> {
    using Grammar::Nonterminal<VarDecl::variant_t>::Nonterminal;
};

struct MethodDeclRule : Grammar::Indexable {
    Type                 type;
    std::string          name;
    FormalList           arguments;
    std::vector<VarDecl> variables;
    std::vector<Stm>     body;
    Exp                  return_exp;

    MethodDeclRule(Builder&&);
};

struct MethodDecl
    : Grammar::Nonterminal<std::variant<MethodDeclRule>> {
    using Grammar::Nonterminal<MethodDecl::variant_t>::Nonterminal;
};

struct ClassDeclNoInheritance : Grammar::Indexable {
    std::string             name;
    std::vector<VarDecl>    variables;
    std::vector<MethodDecl> methods;

    ClassDeclNoInheritance(Builder&&);
};

struct ClassDeclInheritance : Grammar::Indexable {
    std::string             name;
    std::string             superclass;
    std::vector<VarDecl>    variables;
    std::vector<MethodDecl> methods;

    ClassDeclInheritance(Builder&&);
};

// clang-format off
struct ClassDecl : Grammar::Nonterminal<std::variant<
               ClassDeclNoInheritance,
               ClassDeclInheritance>>
{
using Grammar::Nonterminal<ClassDecl::variant_t>::Nonterminal;
};
// clang-format on

struct MainClassRule : Grammar::Indexable {
    std::string name;
    std::string argument;
    Stm         body;

    MainClassRule(Builder&&);
};

struct MainClass : Grammar::Nonterminal<std::variant<MainClassRule>> {
    using Grammar::Nonterminal<MainClass::variant_t>::Nonterminal;
};

struct ProgramRule : Grammar::Indexable {
    MainClass              main;
    std::vector<ClassDecl> classes;

    ProgramRule(Builder&&);
};

struct Program : Grammar::Nonterminal<std::variant<ProgramRule>> {
    using Grammar::Nonterminal<Program::variant_t>::Nonterminal;
};

struct Unexpected {
    Lexeme lex;
};

struct Mismatch {
    Lexeme expected;
    Lexeme found;
};

struct WrongIdentifier {
    std::string expected;
    std::string found;
};

struct ParsingError {
    std::vector<std::string>                            ctx;
    std::vector<int>                                    lines;
    std::variant<Unexpected, Mismatch, WrongIdentifier> inner;
};

using ErrorData = std::vector<std::unique_ptr<ParsingError>>;
} // namespace AST
#endif
