#ifndef BCC_ASTBUILDER
#define BCC_ASTBUILDER
#include "AST.h"
#include "lexer.h"
#include "parser.h"

namespace AST {

class Builder {
    using inner_t =
        std::tuple<std::vector<MainClass>, std::vector<std::string>,
                   std::vector<ClassDecl>, std::vector<VarDecl>,
                   std::vector<MethodDecl>, std::vector<Type>,
                   std::vector<Exp>, std::vector<Stm>,
                   std::vector<FormalList>, std::vector<ExpList>,
                   std::vector<int32_t>>;

    Parser& parser;
    bool    pop = false;

    template <typename T> void _keep(T&& in) {
        using U = std::vector<std::decay_t<T>>;
        std::get<U>(inner).push_back(std::forward<T>(in));
    }

  public:
    int     id;
    inner_t inner;
    Builder(Parser& __parser);
    Builder(Parser& __parser, std::string label);
    ~Builder();
    Builder& operator<<(Lexeme lex);
    Builder& operator<<(std::string in);
    void     unexpected(Lexeme un);

    template <typename T> Builder& operator<<(T&& in) {
        _keep(std::forward<T>(in));
        return *this;
    }

    Exp lhs();
};

template <typename T> std::vector<T> claim(Builder& data) {
    using U = std::vector<std::decay_t<T>>;
    return std::move(std::get<U>(data.inner));
}

template <typename T> T claim(Builder& data, size_t i) {
    using U = std::vector<std::decay_t<T>>;
    return std::move(std::get<U>(data.inner).at(i));
}

} // namespace AST
#endif
