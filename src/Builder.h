#ifndef BCC_ASTBUILDER
#define BCC_ASTBUILDER
#include "AST.h"
#include "lexer.h"

template <typename istream> class Parser;
namespace AST {
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

template <typename istream> class Builder {
    Parser<istream>&             parser;
    Node                         id;
    ptr<MainClass>               main;
    std::vector<std::string>     W;
    std::vector<ptr<ClassDecl>>  C;
    std::vector<ptr<VarDecl>>    V;
    std::vector<ptr<MethodDecl>> M;
    std::vector<ptr<Type>>       T;
    std::vector<ptr<Exp>>        E;
    std::vector<ptr<Stm>>        S;
    ptr<FormalList>              list;
    ptr<ExpList>                 arguments;
    int32_t                      value;
    bool                         pop = false;

  public:
    Builder(Parser<istream>& __parser)
        : parser(__parser), id(parser.idx++) {
        parser.errors.push_back({});
    }

    Builder(Parser<istream>& __parser, std::string label)
        : parser(__parser), id(parser.idx++) {
        parser.errors.push_back({});
        parser.context.push_back(label);
        parser.lines.push_back(parser[0].third);
        pop = true;
    }

    ~Builder() {
        if (pop) {
            parser.context.pop_back();
            parser.lines.pop_back();
        }
    }

    Builder& operator<<(Lexeme lex) {
        if (Lexeme(parser[0]) != lex) {
            auto err   = std::make_unique<ParsingError>();
            err->inner = Mismatch{lex, Lexeme(parser[0])};
            err->ctx   = parser.context;
            parser.errors[id.get()].push_back(std::move(err));
            while (Lexeme(parser[0]) != Lexeme::eof &&
                   Lexeme(parser[0]) != lex)
                ++parser.tokens;
        }
        if (Lexeme::identifier == lex)
            W.push_back(parser[0].second);
        else if (Lexeme::integer_literal == lex)
            value = std::stoi(parser[0].second);
        ++parser.tokens;
        return *this;
    }

    Builder& operator<<(std::string in) {
        if (parser[0].second != in) {
            auto err   = std::make_unique<ParsingError>();
            err->inner = WrongIdentifier{in, parser[0].second};
            err->ctx   = parser.context;
            parser.errors[id.get()].push_back(std::move(err));
            while (Lexeme(parser[0]) != Lexeme::eof &&
                   !(Lexeme(parser[0]) == Lexeme::identifier &&
                     parser[0].second == in))
                ++parser.tokens;
        }
        ++parser.tokens;
        return *this;
    }

    void unexpected(Lexeme un) {
        if (un == Lexeme::eof) return;
        auto err   = std::make_unique<ParsingError>();
        err->inner = Unexpected{un};
        err->ctx   = parser.context;
        parser.errors[id.get()].push_back(std::move(err));
        ++parser.tokens;
    }

    Builder& operator<<(ptr<MainClass>&& in) {
        main.reset(in.release());
        return *this;
    }

    Builder& operator<<(ptr<ClassDecl>&& in) {
        C.push_back(std::move(in));
        return *this;
    }

    Builder& operator<<(ptr<VarDecl>&& in) {
        V.push_back(std::move(in));
        return *this;
    }

    Builder& operator<<(ptr<MethodDecl>&& in) {
        M.push_back(std::move(in));
        return *this;
    }

    Builder& operator<<(ptr<Type>&& in) {
        T.push_back(std::move(in));
        return *this;
    }

    Builder& operator<<(ptr<Exp>&& in) {
        E.push_back(std::move(in));
        return *this;
    }

    Builder& operator<<(ptr<Stm>&& in) {
        S.push_back(std::move(in));
        return *this;
    }

    Builder& operator<<(ptr<ExpList>&& in) {
        arguments.reset(in.release());
        return *this;
    }

    Builder& operator<<(ptr<FormalList>&& in) {
        list.reset(in.release());
        return *this;
    }

    ptr<Program> ProgramRule() {
        return std::make_unique<Program>(
            AST::ProgramRule{id, std::move(main), std::move(C)});
    }

    ptr<MainClass> MainClassRule() {
        return std::make_unique<MainClass>(
            AST::MainClassRule{id, W[0], W[1], std::move(S[0])});
    }

    ptr<ClassDecl> ClassDeclNoInheritance() {
        return std::make_unique<ClassDecl>(
            AST::ClassDeclNoInheritance{id, W[0], std::move(V),
                                        std::move(M)});
    }

    ptr<ClassDecl> ClassDeclInheritance() {
        return std::make_unique<ClassDecl>(AST::ClassDeclInheritance{
            id, W[0], W[1], std::move(V), std::move(M)});
    }

    ptr<MethodDecl> MethodDeclRule() {
        return std::make_unique<MethodDecl>(AST::MethodDeclRule{
            id, std::move(T[0]), W[0], std::move(list), std::move(V),
            std::move(S), std::move(E[0])});
    }

    ptr<VarDecl> VarDeclRule() {
        return std::make_unique<VarDecl>(
            AST::VarDeclRule{id, std::move(T[0]), W[0]});
    }

    ptr<FormalList> FormalListRule() {
        std::vector<FormalDecl> D;
        int s = T.size() < W.size() ? T.size() : W.size();
        for (int i = 0; i < s; i++)
            D.push_back(AST::FormalDecl{std::move(T[i]), W[i]});

        return std::make_unique<FormalList>(
            AST::FormalListRule{id, std::move(D)});
    }

    ptr<Type> integerArrayType() {
        return std::make_unique<Type>(AST::integerArrayType{id});
    }

    ptr<Type> booleanType() {
        return std::make_unique<Type>(AST::booleanType{id});
    }

    ptr<Type> integerType() {
        return std::make_unique<Type>(AST::integerType{id});
    }

    ptr<Type> classType() {
        return std::make_unique<Type>(AST::classType{id, W[0]});
    }

    ptr<ExpList> ExpListRule() {
        return std::make_unique<ExpList>(
            AST::ExpListRule{id, std::move(E)});
    }

    ptr<Stm> blockStm() {
        return std::make_unique<Stm>(AST::blockStm{id, std::move(S)});
    }

    ptr<Stm> ifStm() {
        return std::make_unique<Stm>(AST::ifStm{
            id, std::move(E[0]), std::move(S[0]), std::move(S[1])});
    }

    ptr<Stm> whileStm() {
        return std::make_unique<Stm>(
            AST::whileStm{id, std::move(E[0]), std::move(S[0])});
    }

    ptr<Stm> printStm() {
        return std::make_unique<Stm>(
            AST::printStm{id, std::move(E[0])});
    }

    ptr<Stm> assignStm() {
        return std::make_unique<Stm>(
            AST::assignStm{id, W[0], std::move(E[0])});
    }

    ptr<Stm> indexAssignStm() {
        return std::make_unique<Stm>(AST::indexAssignStm{
            id, W[0], std::move(E[0]), std::move(E[1])});
    }

    ptr<Exp> lhs() { return std::move(E[0]); }

    ptr<Exp> andExp() {
        return std::make_unique<Exp>(
            AST::andExp{id, std::move(E[0]), std::move(E[1])});
    }

    ptr<Exp> lessExp() {
        return std::make_unique<Exp>(
            AST::lessExp{id, std::move(E[0]), std::move(E[1])});
    }

    ptr<Exp> sumExp() {
        return std::make_unique<Exp>(
            AST::sumExp{id, std::move(E[0]), std::move(E[1])});
    }

    ptr<Exp> minusExp() {
        return std::make_unique<Exp>(
            AST::minusExp{id, std::move(E[0]), std::move(E[1])});
    }

    ptr<Exp> prodExp() {
        return std::make_unique<Exp>(
            AST::lessExp{id, std::move(E[0]), std::move(E[1])});
    }

    ptr<Exp> indexingExp() {
        return std::make_unique<Exp>(
            AST::indexingExp{id, std::move(E[0]), std::move(E[1])});
    }

    ptr<Exp> lengthExp() {
        return std::make_unique<Exp>(
            AST::lengthExp{id, std::move(E[0])});
    }

    ptr<Exp> methodCallExp() {
        return std::make_unique<Exp>(AST::methodCallExp{
            id, std::move(E[0]), W[0], std::move(arguments)});
    }

    ptr<Exp> integerExp() {
        return std::make_unique<Exp>(AST::integerExp{id, value});
    }

    ptr<Exp> trueExp() {
        return std::make_unique<Exp>(AST::trueExp{id});
    }

    ptr<Exp> falseExp() {
        return std::make_unique<Exp>(AST::falseExp{id});
    }

    ptr<Exp> thisExp() {
        return std::make_unique<Exp>(AST::thisExp{id});
    }

    ptr<Exp> identifierExp() {
        return std::make_unique<Exp>(AST::identifierExp{id, W[0]});
    }

    ptr<Exp> newArrayExp() {
        return std::make_unique<Exp>(
            AST::newArrayExp{id, std::move(E[0])});
    }

    ptr<Exp> newObjectExp() {
        return std::make_unique<Exp>(AST::newObjectExp{id, W[0]});
    }

    ptr<Exp> bangExp() {
        return std::make_unique<Exp>(
            AST::bangExp{id, std::move(E[0])});
    }
    ptr<Exp> parenExp() {
        return std::make_unique<Exp>(
            AST::parenExp{id, std::move(E[0])});
    }
};
} // namespace AST
#endif
