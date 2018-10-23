#ifndef BCC_LEXER
#define BCC_LEXER

#include <cinttypes>
#include <istream>
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
        Lexeme    label = Lexeme::identifier;
        ptr<Node> child[1 + std::numeric_limits<char>::max()];
    };
    ptr<Node> root;

  public:
    Trie(const std::vector<std::string>& words);
    template <typename It> Lexeme search(It) const;
};

struct LexState {
    Lexeme      first;
    std::string second;
    size_t      third;

    explicit operator Lexeme() { return first; }
};

class Lexer {
    Trie                  symbols;
    std::istream*         in;
    size_t                lo, la, lc, pos;
    std::string           line;
    std::vector<LexState> LA;

    template <typename F> size_t consume(F f);
    LexState                     advance();

  public:
    Lexer(std::istream*, size_t lookahead = 1);
    bool     empty() const;
    LexState operator*();
    LexState operator[](size_t i);
    size_t   line_count() const;
    Lexer&   operator++();
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

template <typename F> size_t Lexer::consume(F f) {
    size_t hi = lo;
    while (hi < line.size() && f(line[hi])) hi++;
    return hi;
}
#endif
