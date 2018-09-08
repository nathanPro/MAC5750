#include "AST.h"
#include "lexer.h"
#include "util.h"

namespace Parser {

struct MismatchError {
    Lexeme expected;
    Lexeme found;
};

struct Unexpected {
    Lexeme lex;
};

struct UnexpectedEnd {};

template <typename istream>
std::string consume(Lexer<istream>& tokens, Lexeme lex) {
    if (tokens.empty()) throw UnexpectedEnd{};
    if ((*tokens).first == lex) {
        std::string ans = (*tokens).second;
        ++tokens;
        return ans;
    } else
        throw MismatchError{lex, (*tokens).first};
}

template <typename istream> AST::ptr<AST::Exp> Exp(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::ExpList> ExpList(Lexer<istream>&);
template <typename istream> AST::ptr<AST::Stm> Stm(Lexer<istream>&);
template <typename istream> AST::ptr<AST::Type> Type(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::FormalList> FormalList(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::VarDecl> VarDecl(Lexer<istream>&);

template <typename istream>
AST::ptr<AST::VarDecl> VarDecl(Lexer<istream>& tokens) {
    auto type = Type(tokens);
    auto word = consume(tokens, Lexeme::identifier);
    consume(tokens, Lexeme::semicolon);
    return std::make_unique<AST::VarDecl>(
        AST::VarDeclRule{std::move(type), word});
}

template <typename istream>
AST::ptr<AST::FormalList> FormalList(Lexer<istream>& tokens) {
    using std::make_unique;
    consume(tokens, Lexeme::open_paren);
    std::vector<AST::FormalDecl> list;
    while ((*tokens).first != Lexeme::close_paren) {
        if (list.size()) consume(tokens, Lexeme::comma);
        auto type = Type(tokens);
        auto word = consume(tokens, Lexeme::identifier);
        list.push_back(AST::FomalDecl{std::move(type), word});
    }
    consume(tokens, Lexeme::close_paren);
    return make_unique<AST::FormalList>(
        AST::FormalListRule{std::move(list)});
}

template <typename istream>
AST::ptr<AST::Type> Type(Lexer<istream>& tokens) {
    using std::make_unique;
    auto [lex, word] = *tokens;
    AST::ptr<AST::Type> ans;
    switch (lex) {
    case Lexeme::boolean_keyword: {
        consume(tokens, Lexeme::boolean_keyword);
        ans = make_unique<AST::Type>(AST::booleanType{});
    } break;
    case Lexeme::identifier: {
        consume(tokens, Lexeme::identifier);
        ans = make_unique<AST::Type>(AST::classType{word});
    } break;
    case Lexeme::int_keyword: {
        consume(tokens, Lexeme::int_keyword);
        if ((*tokens).first == Lexeme::open_bracket) {
            consume(tokens, Lexeme::open_bracket);
            consume(tokens, Lexeme::close_bracket);
            ans = make_unique<AST::Type>(AST::integerArrayType{});
        } else
            ans = make_unique<AST::Type>(AST::integerType{});
    } break;
    default:
        throw Unexpected{lex};
    }
    return ans;
}

template <typename istream>
AST::ptr<AST::Stm> Stm(Lexer<istream>& tokens) {
    debug("calling Stm({", (*tokens).first, ":", (*tokens).second,
          "})");
    using std::make_unique;
    auto [lex, word] = *tokens;
    AST::ptr<AST::Stm> ans;
    switch (lex) {
    case Lexeme::open_brace: {
        std::vector<AST::__detail::pStm> list;
        consume(tokens, Lexeme::open_brace);
        while ((*tokens).first != Lexeme::close_brace)
            list.push_back(Stm(tokens));
        consume(tokens, Lexeme::close_brace);
        ans = make_unique<AST::Stm>(AST::blockStm{std::move(list)});
    } break;
    case Lexeme::if_keyword: {
        consume(tokens, Lexeme::if_keyword);
        consume(tokens, Lexeme::open_paren);
        auto cond = Exp(tokens);
        consume(tokens, Lexeme::close_paren);
        auto if_clause = Stm(tokens);
        consume(tokens, Lexeme::else_keyword);
        auto else_clause = Stm(tokens);
        ans              = make_unique<AST::Stm>(
            AST::ifStm{std::move(cond), std::move(if_clause),
                       std::move(else_clause)});
    } break;
    case Lexeme::while_keyword: {
        consume(tokens, Lexeme::while_keyword);
        consume(tokens, Lexeme::open_paren);
        auto cond = Exp(tokens);
        consume(tokens, Lexeme::close_paren);
        auto body = Stm(tokens);
        ans       = make_unique<AST::Stm>(
            AST::whileStm{std::move(cond), std::move(body)});
    } break;
    case Lexeme::println_keyword: {
        consume(tokens, Lexeme::println_keyword);
        consume(tokens, Lexeme::open_paren);
        auto exp = Exp(tokens);
        consume(tokens, Lexeme::close_paren);
        consume(tokens, Lexeme::semicolon);
        ans = make_unique<AST::Stm>(AST::printStm{std::move(exp)});
    } break;
    case Lexeme::identifier: {
        consume(tokens, Lexeme::identifier);
        if ((*tokens).first == Lexeme::equals_sign) {
            consume(tokens, Lexeme::equals_sign);
            auto exp = Exp(tokens);
            consume(tokens, Lexeme::semicolon);
            ans = make_unique<AST::Stm>(
                AST::assignStm{word, std::move(exp)});
        } else {
            consume(tokens, Lexeme::open_bracket);
            auto idx = Exp(tokens);
            consume(tokens, Lexeme::close_bracket);
            consume(tokens, Lexeme::equals_sign);
            auto exp = Exp(tokens);
            consume(tokens, Lexeme::semicolon);
            ans = make_unique<AST::Stm>(AST::indexAssignStm{
                word, std::move(idx), std::move(exp)});
        }
    } break;
    default:
        throw Unexpected{lex};
    }
    return ans;
}

template <typename istream>
AST::ptr<AST::ExpList> ExpList(Lexer<istream>& tokens) {
    debug("calling ExpList({", (*tokens).first, ":", (*tokens).second,
          "})");
    using std::make_unique;
    auto list = std::vector<AST::__detail::pExp>();
    consume(tokens, Lexeme::open_paren);
    while ((*tokens).first != Lexeme::close_paren) {
        if (list.size()) consume(tokens, Lexeme::comma);
        list.push_back(Exp(tokens));
    }
    consume(tokens, Lexeme::close_paren);
    debug("Finished ExpList");
    return make_unique<AST::ExpList>(
        AST::ExpListRule{std::move(list)});
}

template <typename istream>
AST::ptr<AST::Exp> _Exp(Lexer<istream>& tokens,
                        AST::ptr<AST::Exp>&& lhs) {
    debug("calling _Exp({", (*tokens).first, ":", (*tokens).second,
          "})");
    using std::make_unique;
    auto [lex, word] = *tokens;
    switch (lex) {
    case Lexeme::and_operator: {
        consume(tokens, Lexeme::and_operator);
        auto rhs = Exp(tokens);
        lhs      = make_unique<AST::Exp>(
            AST::andExp{std::move(lhs), std::move(rhs)});
    } break;
    case Lexeme::less_operator: {
        consume(tokens, Lexeme::less_operator);
        auto rhs = Exp(tokens);
        lhs      = make_unique<AST::Exp>(
            AST::lessExp{std::move(lhs), std::move(rhs)});
    } break;
    case Lexeme::plus_operator: {
        consume(tokens, Lexeme::plus_operator);
        auto rhs = Exp(tokens);
        lhs      = make_unique<AST::Exp>(
            AST::sumExp{std::move(lhs), std::move(rhs)});
    } break;
    case Lexeme::minus_operator: {
        consume(tokens, Lexeme::minus_operator);
        auto rhs = Exp(tokens);
        lhs      = make_unique<AST::Exp>(
            AST::minusExp{std::move(lhs), std::move(rhs)});
    } break;
    case Lexeme::times_operator: {
        consume(tokens, Lexeme::times_operator);
        auto rhs = Exp(tokens);
        lhs      = make_unique<AST::Exp>(
            AST::prodExp{std::move(lhs), std::move(rhs)});
    } break;
    case Lexeme::open_bracket: {
        consume(tokens, Lexeme::open_bracket);
        auto index = Exp(tokens);
        consume(tokens, Lexeme::close_bracket);
        lhs = make_unique<AST::Exp>(
            AST::indexingExp{std::move(lhs), std::move(index)});
    } break;
    case Lexeme::period:
        consume(tokens, Lexeme::period);
        if ((*tokens).first == Lexeme::lenght_keyword) {
            consume(tokens, Lexeme::lenght_keyword);
            lhs =
                make_unique<AST::Exp>(AST::lengthExp{std::move(lhs)});
        } else {
            word = (*tokens).second;
            consume(tokens, Lexeme::identifier);
            auto args = ExpList(tokens);
            lhs       = make_unique<AST::Exp>(AST::methodCallExp{
                std::move(lhs), word, std::move(args)});
        }
        break;
    default:
        return std::move(lhs);
    }
    return _Exp(tokens, std::move(lhs));
}

template <typename istream>
AST::ptr<AST::Exp> Exp(Lexer<istream>& tokens) {
    debug("calling Exp({", (*tokens).first, ":", (*tokens).second,
          "})");
    ;
    using std::make_unique;
    auto [lex, word] = *tokens;
    AST::ptr<AST::Exp> lhs;
    switch (lex) {
    case Lexeme::integer_literal: {
        consume(tokens, Lexeme::integer_literal);
        auto value = std::stoi(word);
        lhs        = make_unique<AST::Exp>(AST::integerExp{value});
    } break;
    case Lexeme::true_keyword: {
        consume(tokens, Lexeme::true_keyword);
        lhs = make_unique<AST::Exp>(AST::trueExp{});
    } break;
    case Lexeme::false_keyword: {
        consume(tokens, Lexeme::false_keyword);
        lhs = make_unique<AST::Exp>(AST::falseExp{});
    } break;
    case Lexeme::identifier: {
        consume(tokens, Lexeme::identifier);
        lhs = make_unique<AST::Exp>(AST::identifierExp{word});
    } break;
    case Lexeme::this_keyword: {
        consume(tokens, Lexeme::this_keyword);
        lhs = make_unique<AST::Exp>(AST::thisExp{});
    } break;
    case Lexeme::new_keyword: {
        consume(tokens, Lexeme::new_keyword);
        if ((*tokens).first == Lexeme::int_keyword) {
            consume(tokens, Lexeme::int_keyword);
            consume(tokens, Lexeme::open_bracket);
            auto index = Exp(tokens);
            consume(tokens, Lexeme::close_bracket);
            lhs = make_unique<AST::Exp>(
                AST::newArrayExp{std::move(index)});
        } else {
            std::string name = (*tokens).second;
            consume(tokens, Lexeme::identifier);
            consume(tokens, Lexeme::open_paren);
            consume(tokens, Lexeme::close_paren);
            lhs = make_unique<AST::Exp>(AST::newObjectExp{name});
        }
    } break;
    case Lexeme::bang: {
        consume(tokens, Lexeme::bang);
        auto exp = Exp(tokens);
        lhs = make_unique<AST::Exp>(AST::bangExp{std::move(exp)});
    } break;
    case Lexeme::open_paren: {
        consume(tokens, Lexeme::open_paren);
        auto exp = Exp(tokens);
        consume(tokens, Lexeme::close_paren);
        lhs = make_unique<AST::Exp>(AST::parenExp{std::move(exp)});
    } break;
    default:
        throw Unexpected{lex};
    }
    return _Exp(tokens, std::move(lhs));
}

} // namespace Parser
