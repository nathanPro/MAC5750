#ifndef BCC_LEXER
#define BCC_LEXER

#include <string>
#include <vector>

std::vector<std::string>> reserved_words = {"class",
                                            "public",
                                            "static",
                                            "void",
                                            "String",
                                            "extends",
                                            "return",
                                            "int",
                                            "boolean",
                                            "if",
                                            "else",
                                            "while",
                                            "System.out.println",
                                            "lenght",
                                            "true",
                                            "false",
                                            "this",
                                            "new"};

enum class Lexeme {
    identifier,
    integer_literal,

    and_operator,
    less_operator,
    plus_operator,
    minus_operator,
    times_operator,

    equals_sign,

    class_keyword,
    public_keyword,
    static_keyword,
    void_keyword,
    string_keyword,
    int_keyword,
    boolean_keyword,
    if_keyword,
    else_keyword,
    while_keyword,
    println_keyword,
    lenght_keyword,
    true_keyword,
    false_keyword,
    this_keyword,
    new_keyword,
    return_keyword,
    extends_keyword,

    main_function,
    inline_comment,
    multiline_comment,

    coma,
    semicolon,
    period,
    bang,

    open_paren,
    close_paren,
    open_bracket,
    close_bracket,
    open_brace,
    close_brace
};

template <Lexeme lex, typename V> struct SemanticValue { V value; };

using IdentifierToken =
    SemanticValue<Lexeme::identifier, std::string>;

using IntegerLiteral = 
    SemanticValue<Lexeme::integer_literal, int32_t>;

#endif
