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

    // Hooks for BinaryRule
    ptr<Exp> get(ptr<Exp>, size_t i) { return std::move(E[i]); }

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

    template <typename target, typename ntPtr>
    friend struct AST::__detail::TagRule;
    template <typename target, typename ntPtr>
    friend struct AST::__detail::BinaryRule;
    template <typename target, typename ntPtr>
    friend struct AST::__detail::UnaryRule;
    friend struct ProgramRule;
    friend struct MainClassRule;
    friend struct ClassDeclNoInheritance;
    friend struct ClassDeclInheritance;
    friend struct MethodDeclRule;
    friend struct VarDeclRule;
    friend struct FormalListRule;
    friend struct classType;
    friend struct blockStm;
    friend struct ifStm;
    friend struct whileStm;
    friend struct printStm;
    friend struct assignStm;
    friend struct indexAssignStm;
    friend struct ExpListRule;
    friend struct methodCallExp;
    friend struct integerExp;
    friend struct identifierExp;
    friend struct newObjectExp;

    ptr<Exp> lhs() { return std::move(E[0]); }
};
} // namespace AST
#endif
