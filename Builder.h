#ifndef BUILDER
#define BUILDER
#include "AST.h"

template <typename T> class Builder;

template <> class Builder<AST::ptr<AST::FormalList>> {
    AST::Node id;
    std::vector<AST::ptr<AST::Type>> T;
    std::vector<std::string> W;

  public:
    Builder(AST::Node __id) : id(__id) {}

    Builder& keep(AST::ptr<AST::Type>&& type) {
        T.push_back(std::move(type));
        return *this;
    }

    Builder& keep(std::string name) {
        W.push_back(name);
        return *this;
    }

    AST::ptr<AST::FormalList> FormalListRule() {
        std::vector<AST::FormalDecl> D;
        int s = T.size() < W.size() ? T.size() : W.size();
        for (int i = 0; i < s; i++)
            D.push_back(AST::FormalDecl{std::move(T[i]), W[i]});

        return std::make_unique<AST::FormalList>(
            AST::FormalListRule{id, std::move(D)});
    }
};

template <> class Builder<AST::ptr<AST::Type>> {
    AST::Node id;
    std::string word;

  public:
    Builder(AST::Node __id) : id(__id) {}

    Builder& keep(std::string in) {
        word = in;
        return *this;
    }

    AST::ptr<AST::Type> integerArrayType() {
        return std::make_unique<AST::Type>(AST::integerArrayType{id});
    }

    AST::ptr<AST::Type> booleanType() {
        return std::make_unique<AST::Type>(AST::booleanType{id});
    }

    AST::ptr<AST::Type> integerType() {
        return std::make_unique<AST::Type>(AST::integerType{id});
    }

    AST::ptr<AST::Type> classType() {
        return std::make_unique<AST::Type>(AST::classType{id, word});
    }
};

template <> class Builder<AST::ptr<AST::ExpList>> {
    AST::Node id;
    std::vector<AST::ptr<AST::Exp>> E;

  public:
    Builder(AST::Node __id) : id(__id) {}

    Builder& keep(AST::ptr<AST::Exp>&& exp) {
        E.push_back(std::move(exp));
        return *this;
    }

    AST::ptr<AST::ExpList> ExpListRule() {
        return std::make_unique<AST::ExpList>(
            AST::ExpListRule{id, std::move(E)});
    }
};

template <> class Builder<AST::ptr<AST::Stm>> {
    AST::Node id;
    AST::ptr<AST::Exp> E[2];
    int ec = 0;
    std::vector<AST::ptr<AST::Stm>> S;
    std::string word;

  public:
    Builder(AST::Node __id) : id(__id) {}

    Builder& keep(AST::ptr<AST::Exp>&& exp) {
        E[ec++].reset(exp.release());
        return *this;
    }

    Builder& keep(AST::ptr<AST::Stm>&& stm) {
        S.push_back(std::move(stm));
        return *this;
    }

    Builder& keep(std::string in) {
        word = in;
        return *this;
    }

    AST::ptr<AST::Stm> blockStm() {
        return std::make_unique<AST::Stm>(
            AST::blockStm{id, std::move(S)});
    }

    AST::ptr<AST::Stm> ifStm() {
        return std::make_unique<AST::Stm>(
            AST::ifStm{id, std::move(E[0]), std::move(S.at(0)),
                       std::move(S.at(1))});
    }

    AST::ptr<AST::Stm> whileStm() {
        return std::make_unique<AST::Stm>(
            AST::whileStm{id, std::move(E[0]), std::move(S.at(0))});
    }

    AST::ptr<AST::Stm> printStm() {
        return std::make_unique<AST::Stm>(
            AST::printStm{id, std::move(E[0])});
    }

    AST::ptr<AST::Stm> assignStm() {
        return std::make_unique<AST::Stm>(
            AST::assignStm{id, word, std::move(E[0])});
    }

    AST::ptr<AST::Stm> indexAssignStm() {
        return std::make_unique<AST::Stm>(AST::indexAssignStm{
            id, word, std::move(E[0]), std::move(E[1])});
    }
};

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
