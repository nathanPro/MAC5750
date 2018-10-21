#ifndef AST_H
#define AST_H

#include "grammar.h"
#include "lexer.h"
#include "string"
#include "util.h"
#include <algorithm>
#include <memory>
#include <variant>
#include <vector>
#define UNREACHABLE() (__builtin_unreachable())

namespace AST {
template <typename istream> class Builder;
template <typename T, typename istream>
std::vector<T> claim(Builder<istream>&);

using Node = Entity<struct NodeTag>;

namespace __detail {
struct TagRule {
    Node id;

    template <typename istream>
    TagRule(Builder<istream>&& data) : id(data.id) {}
};

template <typename nt_t> struct BinaryRule {
    Node id;
    nt_t lhs;
    nt_t rhs;

    template <typename istream>
    BinaryRule(Builder<istream>&& data)
        : id(data.id), lhs(claim<nt_t>(data, 0)),
          rhs(claim<nt_t>(data, 1)) {}
};

template <typename nt_t> struct UnaryRule {
    Node id;
    nt_t inner;

    template <typename istream>
    UnaryRule(Builder<istream>&& data)
        : id(data.id), inner(claim<nt_t>(data, 0)) {}
};

template <typename T> struct ValueWrapper {
    Node id;
    T    value;

    template <typename istream>
    ValueWrapper(Builder<istream>&& data)
        : id(data.id), value(claim<T>(data, 0)) {}
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

struct ExpListRule {
    Node             id;
    std::vector<Exp> exps;

    template <typename istream>
    ExpListRule(Builder<istream>&& data)
        : id(data.id), exps(claim<Exp>(data)) {}
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

struct methodCallExp {
    Node        id;
    Exp         object;
    std::string name;
    ExpList     arguments;

    template <typename istream>
    methodCallExp(Builder<istream>&& data)
        : id(data.id), object(claim<Exp>(data, 0)),
          name(claim<std::string>(data, 0)),
          arguments(claim<ExpList>(data, 0)) {}
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

struct blockStm {
    Node             id;
    std::vector<Stm> statements;

    template <typename istream>
    blockStm(Builder<istream>&& data)
        : id(data.id), statements(claim<Stm>(data)) {}
};

struct ifStm {
    Node id;
    Exp  condition;
    Stm  if_clause;
    Stm  else_clause;

    template <typename istream>
    ifStm(Builder<istream>&& data)
        : id(data.id), condition(claim<Exp>(data, 0)),
          if_clause(claim<Stm>(data, 0)),
          else_clause(claim<Stm>(data, 1)) {}
};

struct whileStm {
    Node id;
    Exp  condition;
    Stm  body;

    template <typename istream>
    whileStm(Builder<istream>&& data)
        : id(data.id), condition(claim<Exp>(data, 0)),
          body(claim<Stm>(data, 0)) {}
};

struct printStm {
    Node id;
    Exp  exp;

    template <typename istream>
    printStm(Builder<istream>&& data)
        : id(data.id), exp(claim<Exp>(data, 0)) {}
};

struct assignStm {
    Node        id;
    std::string name;
    Exp         value;

    template <typename istream>
    assignStm(Builder<istream>&& data)
        : id(data.id), name(claim<std::string>(data, 0)),
          value(claim<Exp>(data, 0)) {}
};

struct indexAssignStm {
    Node        id;
    std::string array;
    Exp         index;
    Exp         value;

    template <typename istream>
    indexAssignStm(Builder<istream>&& data)
        : id(data.id), array(claim<std::string>(data, 0)),
          index(claim<Exp>(data, 0)), value(claim<Exp>(data, 1)) {}
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

struct FormalListRule {
    Node                    id;
    std::vector<FormalDecl> decls;

    template <typename istream>
    FormalListRule(Builder<istream>&& data) : id(data.id) {
        std::vector<FormalDecl> D;

        auto T = claim<Type>(data);
        auto W = claim<std::string>(data);

        int s = std::min(T.size(), W.size());
        for (int i = 0; i < s; i++)
            D.push_back(FormalDecl{std::move(T.at(i)), W.at(i)});

        id    = data.id;
        decls = std::move(D);
    }
};

struct FormalList
    : Grammar::Nonterminal<std::variant<FormalListRule>> {
    using Grammar::Nonterminal<FormalList::variant_t>::Nonterminal;
};

struct VarDeclRule {
    Node        id;
    Type        type;
    std::string name;

    template <typename istream>
    VarDeclRule(Builder<istream>&& data)
        : id(data.id), type(claim<Type>(data, 0)),
          name(claim<std::string>(data, 0)) {}
};

struct VarDecl : Grammar::Nonterminal<std::variant<VarDeclRule>> {
    using Grammar::Nonterminal<VarDecl::variant_t>::Nonterminal;
};

struct MethodDeclRule {
    Node                 id;
    Type                 type;
    std::string          name;
    FormalList           arguments;
    std::vector<VarDecl> variables;
    std::vector<Stm>     body;
    Exp                  return_exp;

    template <typename istream>
    MethodDeclRule(Builder<istream>&& data)
        : id(data.id), type(claim<Type>(data, 0)),
          name(claim<std::string>(data, 0)),
          arguments(claim<FormalList>(data, 0)),
          variables(claim<VarDecl>(data)), body(claim<Stm>(data)),
          return_exp(claim<Exp>(data, 0)) {}
};

struct MethodDecl
    : Grammar::Nonterminal<std::variant<MethodDeclRule>> {
    using Grammar::Nonterminal<MethodDecl::variant_t>::Nonterminal;
};

struct ClassDeclNoInheritance {
    Node                    id;
    std::string             name;
    std::vector<VarDecl>    variables;
    std::vector<MethodDecl> methods;

    template <typename istream>
    ClassDeclNoInheritance(Builder<istream>&& data)
        : id(data.id), name(claim<std::string>(data, 0)),
          variables(claim<VarDecl>(data)),
          methods(claim<MethodDecl>(data)) {}
};

struct ClassDeclInheritance {
    Node                    id;
    std::string             name;
    std::string             superclass;
    std::vector<VarDecl>    variables;
    std::vector<MethodDecl> methods;

    template <typename istream>
    ClassDeclInheritance(Builder<istream>&& data)
        : id(data.id), name(claim<std::string>(data, 0)),
          superclass(claim<std::string>(data, 1)),
          variables(claim<VarDecl>(data)),
          methods(claim<MethodDecl>(data)) {}
};

// clang-format off
struct ClassDecl : Grammar::Nonterminal<std::variant<
               ClassDeclNoInheritance,
               ClassDeclInheritance>>
{
using Grammar::Nonterminal<ClassDecl::variant_t>::Nonterminal;
};
// clang-format on

struct MainClassRule {
    Node        id;
    std::string name;
    std::string argument;
    Stm         body;

    template <typename istream>
    MainClassRule(Builder<istream>&& data)
        : id(data.id), name(claim<std::string>(data, 0)),
          argument(claim<std::string>(data, 1)),
          body(claim<Stm>(data, 0)) {}
};

struct MainClass : Grammar::Nonterminal<std::variant<MainClassRule>> {
    using Grammar::Nonterminal<MainClass::variant_t>::Nonterminal;
};

struct ProgramRule {
    Node                   id;
    MainClass              main;
    std::vector<ClassDecl> classes;

    template <typename istream>
    ProgramRule(Builder<istream>&& data)
        : id(data.id), main(claim<MainClass>(data, 0)),
          classes(claim<ClassDecl>(data)) {}
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
    std::variant<Unexpected, Mismatch, WrongIdentifier> inner;
};

using ErrorData = std::vector<std::unique_ptr<ParsingError>>;
} // namespace AST
#endif
