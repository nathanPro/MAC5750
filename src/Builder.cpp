#include "Builder.h"
namespace AST {

Builder::Builder(Parser& __parser)
    : parser(__parser), id(parser.idx++) {
    parser.errors.push_back({});
}

Builder::Builder(Parser& __parser, std::string label)
    : parser(__parser), id(parser.idx++) {
    parser.errors.push_back({});
    parser.record_context(label);
    pop = true;
}

Builder::~Builder() {
    if (pop) parser.drop_context();
}

Builder& Builder::operator<<(Lexeme lex) {
    if (Lexeme(parser[0]) != lex) parser.mismatch(lex, id);
    if (Lexeme::identifier == lex)
        _keep(parser[0].second);
    else if (Lexeme::integer_literal == lex)
        _keep(std::stoi(parser[0].second));
    ++parser.tokens;
    return *this;
}

Builder& Builder::operator<<(std::string in) {
    if (parser[0].second != in) parser.mismatch(in, id);
    ++parser.tokens;
    return *this;
}

void Builder::unexpected(Lexeme un) { parser.unexpected(un, id); }

Exp Builder::lhs() { return std::move(claim<Exp>(*this).at(0)); }

namespace __detail {

TagRule::TagRule(Builder&& data) : Grammar::Indexable{data.id} {}

template <typename nt_t>
BinaryRule<nt_t>::BinaryRule(Builder&& data)
    : Grammar::Indexable{data.id}, lhs(claim<nt_t>(data, 0)),
      rhs(claim<nt_t>(data, 1)) {}

template struct BinaryRule<Exp>::BinaryRule;

template <typename nt_t>
UnaryRule<nt_t>::UnaryRule(Builder&& data)
    : Grammar::Indexable{data.id}, inner(claim<nt_t>(data, 0)) {}
template struct UnaryRule<Exp>::UnaryRule;

template <typename T>
ValueWrapper<T>::ValueWrapper(Builder&& data)
    : Grammar::Indexable{data.id}, value(claim<T>(data, 0)) {}
template struct ValueWrapper<int32_t>::ValueWrapper;
template struct ValueWrapper<std::string>::ValueWrapper;
} // namespace __detail

ExpListRule::ExpListRule(Builder&& data)
    : Grammar::Indexable{data.id}, exps(claim<Exp>(data)) {}

methodCallExp::methodCallExp(Builder&& data)
    : Grammar::Indexable{data.id}, object(claim<Exp>(data, 0)),
      name(claim<std::string>(data, 0)),
      arguments(claim<ExpList>(data, 0)) {}

blockStm::blockStm(Builder&& data)
    : Grammar::Indexable{data.id}, statements(claim<Stm>(data)) {}

ifStm::ifStm(Builder&& data)
    : Grammar::Indexable{data.id}, condition(claim<Exp>(data, 0)),
      if_clause(claim<Stm>(data, 0)),
      else_clause(claim<Stm>(data, 1)) {}

whileStm::whileStm(Builder&& data)
    : Grammar::Indexable{data.id}, condition(claim<Exp>(data, 0)),
      body(claim<Stm>(data, 0)) {}

printStm::printStm(Builder&& data)
    : Grammar::Indexable{data.id}, exp(claim<Exp>(data, 0)) {}

assignStm::assignStm(Builder&& data)
    : Grammar::Indexable{data.id}, name(claim<std::string>(data, 0)),
      value(claim<Exp>(data, 0)) {}

indexAssignStm::indexAssignStm(Builder&& data)
    : Grammar::Indexable{data.id}, array(claim<std::string>(data, 0)),
      index(claim<Exp>(data, 0)), value(claim<Exp>(data, 1)) {}

FormalListRule::FormalListRule(Builder&& data)
    : Grammar::Indexable{data.id} {
    std::vector<FormalDecl> D;

    auto T = claim<Type>(data);
    auto W = claim<std::string>(data);

    int s = std::min(T.size(), W.size());
    for (int i = 0; i < s; i++)
        D.push_back(FormalDecl{std::move(T.at(i)), W.at(i)});

    id    = data.id;
    decls = std::move(D);
}

VarDeclRule::VarDeclRule(Builder&& data)
    : Grammar::Indexable{data.id}, type(claim<Type>(data, 0)),
      name(claim<std::string>(data, 0)) {}

MethodDeclRule::MethodDeclRule(Builder&& data)
    : Grammar::Indexable{data.id}, type(claim<Type>(data, 0)),
      name(claim<std::string>(data, 0)),
      arguments(claim<FormalList>(data, 0)),
      variables(claim<VarDecl>(data)), body(claim<Stm>(data)),
      return_exp(claim<Exp>(data, 0)) {}

ClassDeclNoInheritance::ClassDeclNoInheritance(Builder&& data)
    : Grammar::Indexable{data.id}, name(claim<std::string>(data, 0)),
      variables(claim<VarDecl>(data)),
      methods(claim<MethodDecl>(data)) {}

ClassDeclInheritance::ClassDeclInheritance(Builder&& data)
    : Grammar::Indexable{data.id}, name(claim<std::string>(data, 0)),
      superclass(claim<std::string>(data, 1)),
      variables(claim<VarDecl>(data)),
      methods(claim<MethodDecl>(data)) {}

MainClassRule::MainClassRule(Builder&& data)
    : Grammar::Indexable{data.id}, name(claim<std::string>(data, 0)),
      argument(claim<std::string>(data, 1)),
      body(claim<Stm>(data, 0)) {}

ProgramRule::ProgramRule(Builder&& data)
    : Grammar::Indexable{data.id}, main(claim<MainClass>(data, 0)),
      classes(claim<ClassDecl>(data)) {}
} // namespace AST
