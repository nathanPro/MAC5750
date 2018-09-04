#ifndef BCC_LEXER
#define BCC_LEXER

#include <cinttypes>
#include <limits>
#include <memory>
#include <string>
#include <vector>

// clang-format off
std::vector<std::string> reserved_words = {
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

    coma,
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
    multiline_comment
};

class Trie {
    template <typename T> using ptr = std::unique_ptr<T>;
    struct Node {
        Lexeme label = Lexeme::identifier;
        ptr<Node> child[1 + std::numeric_limits<char>::max()];
    };
    ptr<Node> root;

  public:
    Trie(std::vector<std::string>& words);
    template <typename It> Lexeme search(It) const;
};

template <typename istream> class Lexer {
    using state = std::pair<Lexeme, std::string>;

    Trie symbols;
    std::string line;
    state curr;
    istream& in;
    size_t lo;
    bool fail = false;

    template <typename F> size_t consume(F f);
    state advance();

  public:
    Lexer(istream& _in) : symbols(reserved_words), in(_in), lo(0) {
        curr = advance();
    }

    bool empty() const;
    state operator*();
    Lexer& operator++();
};
#endif
