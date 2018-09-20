#ifndef BUILDER
#define BUILDER
#include "AST.h"

template <typename T> class Builder;

template <> class Builder<AST::ptr<AST::Exp>> {
    AST::Node id;
    AST::ptr<AST::Exp> E[2];
    AST::ptr<AST::ExpList> arguments;
    int32_t value;
    std::string name;
    int ec = 0;

  public:
    Builder(AST::Node __id) : id(__id) {}

    Builder& keep(AST::ptr<AST::Exp>&& exp) {
        E[ec++].reset(exp.release());
        return *this;
    }

    Builder& keep(AST::ptr<AST::ExpList>&& args) {
        arguments.reset(args.release());
        return *this;
    }

    Builder& keep(std::string word) {
        name = word;
        return *this;
    }

    Builder& keep(int32_t val) {
        value = val;
        return *this;
    }

    AST::ptr<AST::Exp> lhs() { return std::move(E[0]); }

    AST::ptr<AST::Exp> andExp() {
        return std::make_unique<AST::Exp>(
            AST::andExp{id, std::move(E[0]), std::move(E[1])});
    }
    AST::ptr<AST::Exp> lessExp() {
        return std::make_unique<AST::Exp>(
            AST::lessExp{id, std::move(E[0]), std::move(E[1])});
    }
    AST::ptr<AST::Exp> sumExp() {
        return std::make_unique<AST::Exp>(
            AST::sumExp{id, std::move(E[0]), std::move(E[1])});
    }
    AST::ptr<AST::Exp> minusExp() {
        return std::make_unique<AST::Exp>(
            AST::minusExp{id, std::move(E[0]), std::move(E[1])});
    }
    AST::ptr<AST::Exp> prodExp() {
        return std::make_unique<AST::Exp>(
            AST::lessExp{id, std::move(E[0]), std::move(E[1])});
    }
    AST::ptr<AST::Exp> indexingExp() {
        return std::make_unique<AST::Exp>(
            AST::indexingExp{id, std::move(E[0]), std::move(E[1])});
    }
    AST::ptr<AST::Exp> lengthExp() {
        return std::make_unique<AST::Exp>(
            AST::lengthExp{id, std::move(E[0])});
    }
    AST::ptr<AST::Exp> methodCallExp() {
        return std::make_unique<AST::Exp>(AST::methodCallExp{
            id, std::move(E[0]), name, std::move(arguments)});
    }
    AST::ptr<AST::Exp> integerExp() {
        return std::make_unique<AST::Exp>(AST::integerExp{id, value});
    }
    AST::ptr<AST::Exp> trueExp() {
        return std::make_unique<AST::Exp>(AST::trueExp{id});
    }
    AST::ptr<AST::Exp> falseExp() {
        return std::make_unique<AST::Exp>(AST::falseExp{id});
    }
    AST::ptr<AST::Exp> thisExp() {
        return std::make_unique<AST::Exp>(AST::thisExp{id});
    }
    AST::ptr<AST::Exp> identifierExp() {
        return std::make_unique<AST::Exp>(
            AST::identifierExp{id, name});
    }
    AST::ptr<AST::Exp> newArrayExp() {
        return std::make_unique<AST::Exp>(
            AST::newArrayExp{id, std::move(E[0])});
    }
    AST::ptr<AST::Exp> newObjectExp() {
        return std::make_unique<AST::Exp>(
            AST::newObjectExp{id, name});
    }
    AST::ptr<AST::Exp> bangExp() {
        return std::make_unique<AST::Exp>(
            AST::bangExp{id, std::move(E[0])});
    }
    AST::ptr<AST::Exp> parenExp() {
        return std::make_unique<AST::Exp>(
            AST::parenExp{id, std::move(E[0])});
    }
};
#endif
