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
    using inner_t = std::tuple<
        std::vector<ptr<MainClass>>, std::vector<std::string>,
        std::vector<ptr<ClassDecl>>, std::vector<ptr<VarDecl>>,
        std::vector<ptr<MethodDecl>>, std::vector<ptr<Type>>,
        std::vector<ptr<Exp>>, std::vector<ptr<Stm>>,
        std::vector<ptr<FormalList>>, std::vector<ptr<ExpList>>,
        std::vector<int32_t>>;

    Parser<istream>& parser;
    bool             pop = false;

    template <typename T> void _keep(T&& in) {
        using U = std::vector<std::decay_t<T>>;
        std::get<U>(inner).push_back(std::forward<T>(in));
    }

  public:
    Node    id;
    inner_t inner;

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
            _keep(parser[0].second);
        else if (Lexeme::integer_literal == lex)
            _keep(std::stoi(parser[0].second));
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

    template <typename T> Builder& operator<<(T&& in) {
        _keep(std::forward<T>(in));
        return *this;
    }

    ptr<Exp> lhs();
};

template <typename T, typename istream>
std::vector<T> claim(Builder<istream>& data) {
    using U = std::vector<std::decay_t<T>>;
    return std::move(std::get<U>(data.inner));
}

template <typename T, typename istream>
T claim(Builder<istream>& data, size_t i) {
    using U = std::vector<std::decay_t<T>>;
    return std::move(std::get<U>(data.inner).at(i));
}

template <typename istream> ptr<Exp> Builder<istream>::lhs() {
    return std::move(claim<ptr<Exp>>(*this).at(0));
}

} // namespace AST
#endif
