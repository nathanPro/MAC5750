#ifndef BCC_LEXER
#define BCC_LEXER

#include <cinttypes>
#include <limits>
#include <memory>
#include <string>
#include <vector>

// clang-format off
const static std::vector<std::string> reserved_words = {
    "boolean",
    "class",
    "else",
    "extends",
    "false",
    "if",
    "int",
    "lenght",
    "new",
    "System.out.println",
    "public",
    "return",
    "static",
    "String",
    "this",
    "true",
    "void",
    "while",
    ",",
    ";",
    ".",
    "!",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    "&&",
    "<",
    "+",
    "-",
    "*",
    "=",
    "//",
    "/*"
};
// clang-format on

// ORDER HERE IS IMPORTANT
enum class Lexeme {
    identifier,
    integer_literal,

    boolean_keyword,
    class_keyword,
    else_keyword,
    extends_keyword,
    false_keyword,
    if_keyword,
    int_keyword,
    lenght_keyword,
    new_keyword,
    println_keyword,
    public_keyword,
    return_keyword,
    static_keyword,
    string_keyword,
    this_keyword,
    true_keyword,
    void_keyword,
    while_keyword,

    comma,
    semicolon,
    period,
    bang,

    open_paren,
    close_paren,
    open_bracket,
    close_bracket,
    open_brace,
    close_brace,

    and_operator,
    less_operator,
    plus_operator,
    minus_operator,
    times_operator,
    equals_sign,

    inline_comment,
    multiline_comment,
    eof
};

template <typename ostream>
ostream& operator<<(ostream& out, Lexeme lex) {
    if (lex == Lexeme::identifier)
        out << "Identifier";
    else if (lex == Lexeme::integer_literal)
        out << "Integer literal";
    else if (lex == Lexeme::eof)
        out << "EOF";
    else
        out << reserved_words[int(lex) - 2];
    return out;
}

class Trie {
    template <typename T> using ptr = std::unique_ptr<T>;
    struct Node {
        Lexeme label = Lexeme::identifier;
        ptr<Node> child[1 + std::numeric_limits<char>::max()];
    };
    ptr<Node> root;

  public:
    Trie(const std::vector<std::string>& words);
    template <typename It> Lexeme search(It) const;
};

template <typename istream> class Lexer {
    using state = std::pair<Lexeme, std::string>;

    Trie symbols;
    istream& in;
    size_t lo, la, pos;
    std::string line;
    std::vector<state> LA;

    template <typename F> size_t consume(F f);
    state advance();

  public:
    Lexer(istream&, size_t lookahead = 1);
    bool empty() const;
    state operator*();
    state operator[](size_t i);
    Lexer& operator++();
};

template <typename It> Lexeme Trie::search(It q) const {
    Node* node = root.get();
    while (*q) {
        if (node->child[static_cast<int>(*q)])
            node = node->child[static_cast<int>(*q++)].get();
        else
            return Lexeme::identifier;
    }
    if (!node->child[0]) return Lexeme::identifier;
    return node->child[0]->label;
}

template <typename istream>
Lexer<istream>::Lexer(istream& _in, size_t _la)
    : symbols(reserved_words), in(_in), lo(0), la(_la), pos(0),
      LA(la) {
    for (size_t i = 0; i < la; i++) LA[i] = advance();
}

template <typename istream>
typename Lexer<istream>::state Lexer<istream>::operator[](size_t i) {
    return LA[(pos + i) % la];
}

template <typename istream>
template <typename F>
size_t Lexer<istream>::consume(F f) {
    size_t hi = lo;
    while (hi < line.size() && f(line[hi])) hi++;
    return hi;
}

template <typename istream>
typename Lexer<istream>::state Lexer<istream>::advance() {
    if (LA[pos].first == Lexeme::eof) return LA[pos];
    while (true) {
        if (lo == line.size()) {
            std::getline(in, line);
            lo = 0;
            if (!in.good()) return {Lexeme::eof, std::string()};
        }
        while (lo < line.size()) {
            while (lo < line.size() && isspace(line[lo])) lo++;
            if (lo == line.size()) break;

            if (ispunct(line[lo])) {
                // Still need to fix /* */ comment style
                std::string word(line, lo, 2);
                auto lex = symbols.search(word.c_str());
                if (lex == Lexeme::identifier) {
                    word = std::string(line, lo, 1);
                    lex  = symbols.search(word.c_str());
                } else if (lex == Lexeme::inline_comment) {
                    lo = line.size();
                    continue; // NON-OBVIOUS CONTROL FLOW
                }
                lo += word.size();
                return {lex, word};
            }

            if (isdigit(line[lo])) {
                size_t hi =
                    consume([](char c) { return isdigit(c); });
                std::string word(line, lo, hi - lo);
                lo += word.size();
                return {Lexeme::integer_literal, word};
            }

            if (isalpha(line[lo])) {
                std::string word(line, lo, 18);
                auto lex = symbols.search(word.c_str());
                if (lex == Lexeme::println_keyword) {
                    lo += 18;
                    return {lex, word};
                }

                size_t hi = consume(
                    [](char c) { return isalnum(c) || c == '_'; });
                word = std::string(line, lo, hi - lo);
                lex  = symbols.search(word.c_str());
                lo += word.size();
                return {lex, word};
            }
            __builtin_unreachable();
        }
    }
}

template <typename istream> bool Lexer<istream>::empty() const {
    return LA[pos].first == Lexeme::eof;
}

template <typename istream>
typename Lexer<istream>::state Lexer<istream>::operator*() {
    return LA[pos];
}

template <typename istream>
Lexer<istream>& Lexer<istream>::operator++() {
    LA[pos] = advance();
    pos     = (pos + 1) % la;
    return *this;
}
#endif
