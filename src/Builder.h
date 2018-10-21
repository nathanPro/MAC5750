#ifndef BCC_ASTBUILDER
#define BCC_ASTBUILDER
#include "AST.h"
#include "lexer.h"

template <typename istream> class Parser;
namespace AST {

template <typename istream> class Builder {
    using inner_t =
        std::tuple<std::vector<MainClass>, std::vector<std::string>,
                   std::vector<ClassDecl>, std::vector<VarDecl>,
                   std::vector<MethodDecl>, std::vector<Type>,
                   std::vector<Exp>, std::vector<Stm>,
                   std::vector<FormalList>, std::vector<ExpList>,
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
        parser.record_context(label);
        pop = true;
    }

    ~Builder() {
        if (pop) parser.drop_context();
    }

    Builder& operator<<(Lexeme lex) {
        if (Lexeme(parser[0]) != lex) parser.mismatch(lex, id);
        if (Lexeme::identifier == lex)
            _keep(parser[0].second);
        else if (Lexeme::integer_literal == lex)
            _keep(std::stoi(parser[0].second));
        ++parser.tokens;
        return *this;
    }

    Builder& operator<<(std::string in) {
        if (parser[0].second != in) parser.mismatch(in, id);
        ++parser.tokens;
        return *this;
    }

    void unexpected(Lexeme un) { parser.unexpected(un, id); }

    template <typename T> Builder& operator<<(T&& in) {
        _keep(std::forward<T>(in));
        return *this;
    }

    Exp lhs();
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

template <typename istream> Exp Builder<istream>::lhs() {
    return std::move(claim<Exp>(*this).at(0));
}

} // namespace AST
#endif
