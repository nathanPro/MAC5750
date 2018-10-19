#ifndef AST_H
#define AST_H

#include "grammar.h"
#include "string"
#include "util.h"
#include <algorithm>
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

template <typename istream> class Builder;
template <typename T, typename istream>
std::vector<T> claim(Builder<istream>&);

using Node = Entity<struct NodeTag>;

namespace __detail {
template <typename target, typename ntPtr> struct TagRule {
    using ptr = typename std::pointer_traits<ntPtr>::pointer;
    using elm = typename std::pointer_traits<ntPtr>::element_type;

    Node id;

    template <typename istream>
    static ptr build(Builder<istream>&& data) {
        return std::make_unique<elm>(target{data.id});
    }
};

template <typename target, typename ntPtr> struct BinaryRule {
    using ptr = typename std::pointer_traits<ntPtr>::pointer;
    using elm = typename std::pointer_traits<ntPtr>::element_type;

    Node id;
    ptr  lhs = nullptr;
    ptr  rhs = nullptr;

    template <typename istream>
    static ptr build(Builder<istream>&& data) {
        auto tmp = claim<ptr>(data);
        return std::make_unique<elm>(target{
            data.id, std::move(tmp.at(0)), std::move(tmp.at(1))});
    }
};

template <typename target, typename ntPtr> struct UnaryRule {
    using ptr = typename std::pointer_traits<ntPtr>::pointer;
    using elm = typename std::pointer_traits<ntPtr>::element_type;

    Node id;
    ptr  inner = nullptr;

    template <typename istream>
    static ptr build(Builder<istream>&& data) {
        auto tmp = claim<ptr>(data);
        return std::make_unique<elm>(
            target{data.id, std::move(tmp.at(0))});
    }
};

template <typename target, typename T, typename ntPtr>
struct ValueWrapper {
    using ptr = typename std::pointer_traits<ntPtr>::pointer;
    using elm = typename std::pointer_traits<ntPtr>::element_type;

    Node id;
    T    value;

    template <typename istream>
    static ptr build(Builder<istream>&& data) {
        auto tmp = claim<T>(data);
        return std::make_unique<elm>(
            target{data.id, std::move(tmp.at(0))});
    }
};
} // namespace __detail

struct ProgramRule {
    Node                        id;
    ptr<MainClass>              main = nullptr;
    std::vector<ptr<ClassDecl>> classes;

    template <typename istream>
    ProgramRule(Builder<istream>&& data)
        : id(data.id), main(claim<ptr<MainClass>>(data, 0)),
          classes(claim<ptr<ClassDecl>>(data)) {}

    template <typename istream>
    static ptr<Program> build(Builder<istream>&& data) {
        return std::make_unique<Program>(
            ProgramRule(std::move(data)));
    }
};

struct Program : Grammar::Nonterminal<std::variant<ProgramRule>> {
    using Grammar::Nonterminal<Program::variant_t>::Nonterminal;
};

struct MainClassRule {
    Node        id;
    std::string name;
    std::string argument;
    ptr<Stm>    body = nullptr;

    template <typename istream>
    static ptr<MainClass> build(Builder<istream>&& data) {
        auto words = claim<std::string>(data);
        auto stms  = claim<ptr<Stm>>(data);
        return std::make_unique<MainClass>(
            MainClassRule{data.id, words.at(0), words.at(1),
                          std::move(stms.at(0))});
    }
};

struct MainClass : Grammar::Nonterminal<std::variant<MainClassRule>> {
    using Grammar::Nonterminal<MainClass::variant_t>::Nonterminal;
};

struct ClassDeclNoInheritance {
    Node                         id;
    std::string                  name;
    std::vector<ptr<VarDecl>>    variables;
    std::vector<ptr<MethodDecl>> methods;

    template <typename istream>
    static ptr<ClassDecl> build(Builder<istream>&& data) {
        auto w = claim<std::string>(data);
        return std::make_unique<ClassDecl>(ClassDeclNoInheritance{
            data.id, w.at(0), claim<ptr<VarDecl>>(data),
            claim<ptr<MethodDecl>>(data)});
    }
};

struct ClassDeclInheritance {
    Node                         id;
    std::string                  name;
    std::string                  superclass;
    std::vector<ptr<VarDecl>>    variables;
    std::vector<ptr<MethodDecl>> methods;

    template <typename istream>
    static ptr<ClassDecl> build(Builder<istream>&& data) {
        auto W = claim<std::string>(data);
        return std::make_unique<ClassDecl>(ClassDeclInheritance{
            data.id, W.at(0), W.at(1), claim<ptr<VarDecl>>(data),
            claim<ptr<MethodDecl>>(data)});
    }
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
    Node        id;
    ptr<Type>   type = nullptr;
    std::string name;

    template <typename istream>
    static ptr<VarDecl> build(Builder<istream>&& data) {
        auto T = claim<ptr<Type>>(data);
        auto W = claim<std::string>(data);
        return std::make_unique<VarDecl>(VarDeclRule{
            data.id, std::move(T.at(0)), std::move(W.at(0))});
    }
};

struct VarDecl : Grammar::Nonterminal<std::variant<VarDeclRule>> {
    using Grammar::Nonterminal<VarDecl::variant_t>::Nonterminal;
};

struct MethodDeclRule {
    Node                      id;
    ptr<Type>                 type = nullptr;
    std::string               name;
    ptr<FormalList>           arguments;
    std::vector<ptr<VarDecl>> variables;
    std::vector<ptr<Stm>>     body;
    ptr<Exp>                  return_exp = nullptr;

    template <typename istream>
    static ptr<MethodDecl> build(Builder<istream>&& data) {
        auto E    = claim<ptr<Exp>>(data);
        auto T    = claim<ptr<Type>>(data);
        auto W    = claim<std::string>(data);
        auto list = claim<ptr<FormalList>>(data);
        return std::make_unique<MethodDecl>(MethodDeclRule{
            data.id, std::move(T.at(0)), W.at(0),
            std::move(list.at(0)), claim<ptr<VarDecl>>(data),
            claim<ptr<Stm>>(data), std::move(E.at(0))});
    }
};

struct MethodDecl
    : Grammar::Nonterminal<std::variant<MethodDeclRule>> {
    using Grammar::Nonterminal<MethodDecl::variant_t>::Nonterminal;
};

struct FormalDecl {
    ptr<Type>   type = nullptr;
    std::string name;
};

struct FormalListRule {
    Node                    id;
    std::vector<FormalDecl> decls;

    template <typename istream>
    static ptr<FormalList> build(Builder<istream>&& data) {
        std::vector<FormalDecl> D;

        auto T = claim<ptr<Type>>(data);
        auto W = claim<std::string>(data);

        int s = std::min(T.size(), W.size());
        for (int i = 0; i < s; i++)
            D.push_back(FormalDecl{std::move(T.at(i)), W.at(i)});

        return std::make_unique<FormalList>(
            FormalListRule{data.id, std::move(D)});
    }
};

struct FormalList
    : Grammar::Nonterminal<std::variant<FormalListRule>> {
    using Grammar::Nonterminal<FormalList::variant_t>::Nonterminal;
};

struct integerArrayType
    : __detail::TagRule<integerArrayType, ptr<Type>> {};
struct booleanType : __detail::TagRule<booleanType, ptr<Type>> {};
struct integerType : __detail::TagRule<integerType, ptr<Type>> {};
struct classType
    : __detail::ValueWrapper<classType, std::string, ptr<Type>> {};

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
    Node                  id;
    std::vector<ptr<Stm>> statements;

    template <typename istream>
    static ptr<Stm> build(Builder<istream>&& data) {
        return std::make_unique<Stm>(
            blockStm{data.id, claim<ptr<Stm>>(data)});
    }
};

struct ifStm {
    Node     id;
    ptr<Exp> condition   = nullptr;
    ptr<Stm> if_clause   = nullptr;
    ptr<Stm> else_clause = nullptr;

    template <typename istream>
    static ptr<Stm> build(Builder<istream>&& data) {
        auto E = claim<ptr<Exp>>(data);
        auto S = claim<ptr<Stm>>(data);
        return std::make_unique<Stm>(
            ifStm{data.id, std::move(E.at(0)), std::move(S.at(0)),
                  std::move(S.at(1))});
    }
};

struct whileStm {
    Node     id;
    ptr<Exp> condition = nullptr;
    ptr<Stm> body;

    template <typename istream>
    static ptr<Stm> build(Builder<istream>&& data) {
        auto E = claim<ptr<Exp>>(data);
        auto S = claim<ptr<Stm>>(data);
        return std::make_unique<Stm>(whileStm{
            data.id, std::move(E.at(0)), std::move(S.at(0))});
    }
};

struct printStm {
    Node     id;
    ptr<Exp> exp = nullptr;

    template <typename istream>
    static ptr<Stm> build(Builder<istream>&& data) {
        auto E = claim<ptr<Exp>>(data);
        return std::make_unique<Stm>(
            printStm{data.id, std::move(E.at(0))});
    };
};

struct assignStm {
    Node        id;
    std::string name;
    ptr<Exp>    value = nullptr;

    template <typename istream>
    static ptr<Stm> build(Builder<istream>&& data) {
        auto W = claim<std::string>(data);
        auto E = claim<ptr<Exp>>(data);
        return std::make_unique<Stm>(
            assignStm{data.id, W.at(0), std::move(E.at(0))});
    }
};

struct indexAssignStm {
    Node        id;
    std::string array;
    ptr<Exp>    index = nullptr;
    ptr<Exp>    value = nullptr;

    template <typename istream>
    static ptr<Stm> build(Builder<istream>&& data) {
        auto W = claim<std::string>(data);
        auto E = claim<ptr<Exp>>(data);
        return std::make_unique<Stm>(
            indexAssignStm{data.id, W.at(0), std::move(E.at(0)),
                           std::move(E.at(1))});
    }
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

struct andExp : __detail::BinaryRule<andExp, ptr<Exp>> {};
struct lessExp : __detail::BinaryRule<lessExp, ptr<Exp>> {};
struct sumExp : __detail::BinaryRule<sumExp, ptr<Exp>> {};
struct minusExp : __detail::BinaryRule<minusExp, ptr<Exp>> {};
struct prodExp : __detail::BinaryRule<prodExp, ptr<Exp>> {};
struct indexingExp : __detail::BinaryRule<indexingExp, ptr<Exp>> {};
struct lengthExp : __detail::UnaryRule<lengthExp, ptr<Exp>> {};

struct methodCallExp {
    Node         id;
    ptr<Exp>     object = nullptr;
    std::string  name;
    ptr<ExpList> arguments = nullptr;

    template <typename istream>
    static ptr<Exp> build(Builder<istream>&& data) {
        auto E  = claim<ptr<Exp>>(data);
        auto W  = claim<std::string>(data);
        auto El = claim<ptr<ExpList>>(data);
        return std::make_unique<Exp>(
            AST::methodCallExp{data.id, std::move(E.at(0)), W.at(0),
                               std::move(El.at(0))});
    }
};

struct integerExp
    : __detail::ValueWrapper<integerExp, int32_t, ptr<Exp>> {};
struct identifierExp
    : __detail::ValueWrapper<identifierExp, std::string, ptr<Exp>> {};
struct newObjectExp
    : __detail::ValueWrapper<newObjectExp, std::string, ptr<Exp>> {};

struct trueExp : __detail::TagRule<trueExp, ptr<Exp>> {};
struct falseExp : __detail::TagRule<falseExp, ptr<Exp>> {};
struct thisExp : __detail::TagRule<thisExp, ptr<Exp>> {};

struct newArrayExp : __detail::UnaryRule<newArrayExp, ptr<Exp>> {};

struct bangExp : __detail::UnaryRule<bangExp, ptr<Exp>> {};
struct parenExp : __detail::UnaryRule<parenExp, ptr<Exp>> {};

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
    Node                  id;
    std::vector<ptr<Exp>> exps;

    template <typename istream>
    static ptr<ExpList> build(Builder<istream>&& data) {
        return std::make_unique<ExpList>(
            ExpListRule{data.id, claim<ptr<Exp>>(data)});
    }
};

struct ExpList : Grammar::Nonterminal<std::variant<ExpListRule>> {
    using Grammar::Nonterminal<ExpList::variant_t>::Nonterminal;
};

} // namespace AST
#endif
