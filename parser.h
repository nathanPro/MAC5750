#include "AST.h"
#include "lexer.h"
#include "util.h"

namespace Parser {
template <typename istream>
std::string consume(Lexer<istream>& tokens, Lexeme lex) {
    if ((*tokens).first == lex) {
        std::string ans = (*tokens).second;
        ++tokens;
        return ans;
    } else
        throw;
}

template <typename istream>
AST::ptr<AST::Exp> _Exp(Lexer<istream>& tokens,
                        AST::ptr<AST::Exp>&& lhs) {
    auto [lex, word] = *tokens;
    switch (lex) {
    case Lexeme::and_operator:
        consume(tokens, Lexeme::and_operator);
        auto rhs = Exp(tokens);
        lhs      = std::make_unique<AST::Exp>(
            AST::andExp{std::move(lhs), std::move(rhs)});
        break;
    case Lexeme::less_operator:
        consume(tokens, Lexeme::less_operator);
        auto rhs = Exp(tokens);
        lhs      = std::make_unique<AST::Exp>(
            AST::lessExp{std::move(lhs), std::move(rhs)});
        break;
    case Lexeme::plus_operator:
        consume(tokens, Lexeme::plus_operator);
        auto rhs = Exp(tokens);
        lhs      = std::make_unique<AST::Exp>(
            AST::sumExp{std::move(lhs), std::move(rhs)});
        break;
    case Lexeme::minus_operator:
        consume(tokens, Lexeme::minus_operator);
        auto rhs = Exp(tokens);
        lhs      = std::make_unique<AST::Exp>(
            AST::minusExp{std::move(lhs), std::move(rhs)});
        break;
    case Lexeme::times_operator:
        consume(tokens, Lexeme::times_operator);
        auto rhs = Exp(tokens);
        lhs      = std::make_unique<AST::Exp>(
            AST::prodExp{std::move(lhs), std::move(rhs)});
        break;
    case Lexeme::open_bracket:
        consume(tokens, Lexeme::open_bracket);
        auto index = Exp(tokens);
        consume(tokens, Lexeme::close_bracket);
        lhs = std::make_unique<AST::Exp>(
            AST::indexingExp{std::move(lhs), std::move(index)});
        break;
    case Lexeme::period:
        consume(tokens, Lexeme::period);
        if ((*tokens).first == Lexeme::lenght_keyword) {
            lhs = std::make_unique<AST::Exp>(
                AST::lengthExp{std::move(lhs)});
            ++tokens;
        } else {
            word = (*tokens).second;
            consume(tokens, Lexeme::open_paren);
            auto args = ExpList(tokens);
            consume(tokens, Lexeme::close_paren);
            lhs = std::make_unique<AST::Exp>(
                AST::methodCallExp{std::move(lhs), word, args});
        }
        break;
    default:
        return lhs;
    }
    return _Exp(tokens, std::move(lhs));
}

template <typename istream>
AST::ptr<AST::Exp> Exp(Lexer<istream>& tokens) {
    auto [lex, word] = *tokens;
    AST::ptr<AST::Exp> lhs;
    switch (lex) {
    case Lexeme::integer_literal:
        consume(tokens, Lexeme::integer_literal);
        auto value = std::stoi(word);
        lhs        = make_unique<AST::Exp>(AST::integerExp{value});
        break;
    case Lexeme::true_keyword:
        consume(tokens, Lexeme::true_keyword);
        lhs = make_unique<AST::Exp>(AST::trueExp{});
        break;
    case Lexeme::false_keyword:
        consume(tokens, Lexeme::false_keyword);
        lhs = make_unique<AST::Exp>(AST::falseExp{});
        break;
    case Lexeme::identifier:
        consume(tokens, Lexeme::identifier);
        lhs = make_unique<AST::Exp>(AST::identifierExp{word});
        break;
    case Lexeme::this_keyword:
        consume(tokens, Lexeme::this_keyword);
        lhs = make_unique<AST::Exp>(AST::thisExp{});
        break;
    case Lexeme::new_keyword:
        consume(tokens, Lexeme::new_keyword);
        if ((*tokens).first == Lexeme::int_keyword) {
            consume(tokens, Lexeme::int_keyword);
            consume(tokens, Lexeme::open_bracket);
            auto index = Exp(tokens);
            consume(tokens, Lexeme::close_bracket);
            lhs =
                make_unique<AST::Exp>(newArrayExp{std::move(index)});
        } else {
            std::string name = (*token).second;
            consume(tokens, Lexeme::identifier);
            consume(tokens, Lexeme::open_paren);
            consume(tokens, Lexeme::close_paren);
            lhs = make_unique<AST::Exp>(newObjectExp{name});
        }
        break;
    case Lexeme::bang:
        consume(tokens, Lexeme::bang);
        auto exp = Exp(tokens);
        lhs = make_unique<AST::Exp>(AST::bangExp{std::move(exp)});
        break;
    case Lexeme::open_paren:
        consume(tokens, Lexeme::open_paren);
        auto exp = Exp(tokens);
        consume(tokens, Lexeme::close_paren);
        lhs = make_unique<AST::Exp>(AST::parenExp{std::move(exp)});
        break;
    default:
        throw;
    }
    return _Exp(tokens, std::move(lhs));
}
} // namespace Parser
