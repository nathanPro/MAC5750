#include "AST.h"
#include "Builder.h"
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

template <typename istream> class Parser {
    Lexer<istream> tokens;
    int idx;

  public:
    Parser(istream& stream) : tokens(stream, 2), idx(0) {}
    AST::Node make_id() { return AST::Node(idx++); }
    LexState operator[](int i) { return tokens[i]; }
    std::string consume(Lexeme lex) {
        if (tokens.empty()) throw UnexpectedEnd{};
        if ((*tokens).first == lex) {
            std::string ans = (*tokens).second;
            ++tokens;
            return ans;
        } else {
            write(std::cerr, "ERROR:");
            std::cerr << "Expected '" << lex << "' and found '"
                      << tokens[0].first << "'\n";
            throw MismatchError{lex, tokens[0].first};
        }
    }
};

struct ContextGuard {
    size_t line;
    std::string label;
    bool active;

    ContextGuard(size_t _line, std::string _label)
        : line(_line), label(_label), active(true) {}
    ~ContextGuard() {
        if (!active) return;
        write(std::cerr, "while parsing", label, "from line", line);
    }
};

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
AST::ptr<AST::MethodDecl> MethodDecl(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::ClassDecl> ClassDecl(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::MainClass> MainClass(Lexer<istream>&);
template <typename istream>
AST::ptr<AST::Program> Program(Lexer<istream>&);

template <typename istream>
AST::ptr<AST::Program> Program(Parser<istream>& parser) {
    Builder<AST::ptr<AST::Program>> builder(parser.make_id());
    builder.keep(MainClass(parser));
    while (Lexeme(parser[0]) != Lexeme::eof)
        builder.keep(ClassDecl(parser));
    return builder.ProgramRule();
}

template <typename istream>
AST::ptr<AST::MainClass> MainClass(Parser<istream>& parser) {
    Builder<AST::ptr<AST::MainClass>> builder(parser.make_id());
    parser.consume(Lexeme::class_keyword);
    builder.keep(parser.consume(Lexeme::identifier));
    parser.consume(Lexeme::open_brace);
    parser.consume(Lexeme::public_keyword);
    parser.consume(Lexeme::static_keyword);
    parser.consume(Lexeme::void_keyword);
    if (parser[0].second != std::string("main"))
        throw Unexpected{Lexeme(parser[0])};
    parser.consume(Lexeme::identifier);
    parser.consume(Lexeme::open_paren);
    parser.consume(Lexeme::string_keyword);
    parser.consume(Lexeme::open_bracket);
    parser.consume(Lexeme::close_bracket);
    builder.keep(parser.consume(Lexeme::identifier));
    parser.consume(Lexeme::close_paren);
    parser.consume(Lexeme::open_brace);
    builder.keep(Stm(parser));
    parser.consume(Lexeme::close_brace);
    parser.consume(Lexeme::close_brace);
    return builder.MainClassRule();
}

template <typename istream>
AST::ptr<AST::ClassDecl> ClassDecl(Parser<istream>& parser) {
    Builder<AST::ptr<AST::MainClass>> builder(parser.make_id());
    parser.consume(Lexeme::class_keyword);
    builder.keep(parser.consume(Lexeme::identifier));

    bool has_superclass =
        (Lexeme(parser[0]) == Lexeme::extends_keyword);

    if (has_superclass) {
        parser.consume(Lexeme::extends_keyword);
        builder.keep(parser.consume(Lexeme::identifier));
    }

    parser.consume(Lexeme::open_brace);
    while (Lexeme(parser[0]) != Lexeme::close_brace &&
           Lexeme(parser[0]) != Lexeme::public_keyword)
        builder.keep(VarDecl(parser));

    while (Lexeme(parser[0]) != Lexeme::close_brace)
        builder.keep(MethodDecl(parser));
    parser.consume(Lexeme::close_brace);

    if (has_superclass)
        return builder.ClassDeclInheritance();
    else
        return builder.ClassDeclNoInheritance();
}

template <typename istream>
AST::ptr<AST::MethodDecl> MethodDecl(Parser<istream>& parser) {
    Builder<AST::ptr<AST::MethodDecl>> builder(parser.make_id());
    parser.consume(Lexeme::public_keyword);
    builder.keep(Type(parser))
        .keep(parser.consume(Lexeme::identifier))
        .keep(FormalList(parser));
    parser.consume(Lexeme::open_brace);

    bool eating_vars = true;
    while (eating_vars) {
        switch (Lexeme(parser[0])) {
        case Lexeme::boolean_keyword:
        case Lexeme::int_keyword:
            builder.keep(VarDecl(parser));
            break;
        case Lexeme::identifier:
            if (Lexeme(parser[1]) == Lexeme::identifier) {
                builder.keep(VarDecl(parser));
                break;
            }
        default:
            eating_vars = false;
        }
    }

    while (Lexeme(parser[0]) != Lexeme::return_keyword)
        builder.keep(Stm(parser));
    parser.consume(Lexeme::return_keyword);
    builder.keep(Exp(parser));
    parser.consume(Lexeme::semicolon);
    parser.consume(Lexeme::close_brace);
    return builder.MethodDeclRule();
}

template <typename istream>
AST::ptr<AST::VarDecl> VarDecl(Parser<istream>& parser) {
    ContextGuard guard(parser[0].third, "<variable declaration>");
    auto type = Type(parser);
    auto word = parser.consume(Lexeme::identifier);
    parser.consume(Lexeme::semicolon);
    guard.active = false;
    return std::make_unique<AST::VarDecl>(
        AST::VarDeclRule{parser.make_id(), std::move(type), word});
}

template <typename istream>
AST::ptr<AST::FormalList> FormalList(Parser<istream>& parser) {
    Builder<AST::ptr<AST::FormalList>> builder(parser.make_id());
    parser.consume(Lexeme::open_paren);
    bool first = true;
    while (Lexeme(parser[0]) != Lexeme::close_paren) {
        if (!first) parser.consume(Lexeme::comma);
        first = false;
        builder.keep(Type(parser))
            .keep(parser.consume(Lexeme::identifier));
    }
    parser.consume(Lexeme::close_paren);
    return builder.FormalListRule();
}

template <typename istream>
AST::ptr<AST::Type> Type(Parser<istream>& parser) {
    Builder<AST::ptr<AST::Type>> builder(parser.make_id());
    switch (Lexeme(parser[0])) {
    case Lexeme::boolean_keyword:
        parser.consume(Lexeme::boolean_keyword);
        return builder.booleanType();
    case Lexeme::identifier:
        return builder.keep(parser.consume(Lexeme::identifier))
            .classType();
    case Lexeme::int_keyword:
        parser.consume(Lexeme::int_keyword);
        if (Lexeme(parser[0]) == Lexeme::open_bracket) {
            parser.consume(Lexeme::open_bracket);
            parser.consume(Lexeme::close_bracket);
            return builder.integerArrayType();
        }
        return builder.integerType();
    default:
        throw Unexpected{Lexeme(parser[0])};
    }
}

template <typename istream>
AST::ptr<AST::Stm> Stm(Parser<istream>& parser) {
    Builder<AST::ptr<AST::Stm>> builder(parser.make_id());
    switch (Lexeme(parser[0])) {
    case Lexeme::open_brace:
        parser.consume(Lexeme::open_brace);
        while (Lexeme(parser[0]) != Lexeme::close_brace)
            builder.keep(Stm(parser));
        parser.consume(Lexeme::close_brace);
        return builder.blockStm();
    case Lexeme::if_keyword:
        parser.consume(Lexeme::if_keyword);
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        builder.keep(Stm(parser));
        parser.consume(Lexeme::else_keyword);
        return builder.keep(Stm(parser)).ifStm();
    case Lexeme::while_keyword:
        parser.consume(Lexeme::while_keyword);
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        return builder.keep(Stm(parser)).whileStm();
    case Lexeme::println_keyword:
        parser.consume(Lexeme::println_keyword);
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        parser.consume(Lexeme::semicolon);
        return builder.printStm();
    case Lexeme::identifier:
        builder.keep(parser.consume(Lexeme::identifier));
        if (Lexeme(parser[0]) == Lexeme::equals_sign) {
            parser.consume(Lexeme::equals_sign);
            builder.keep(Exp(parser));
            parser.consume(Lexeme::semicolon);
            return builder.assignStm();
        }
        parser.consume(Lexeme::open_bracket);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_bracket);
        parser.consume(Lexeme::equals_sign);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::semicolon);
        return builder.indexAssignStm();
    default:
        throw Unexpected{Lexeme(parser[0])};
    }
}

template <typename istream>
AST::ptr<AST::ExpList> ExpList(Parser<istream>& parser) {
    Builder<AST::ptr<AST::ExpList>> builder(parser.make_id());
    parser.consume(Lexeme::open_paren);
    bool read = false;
    while (Lexeme(parser[0]) != Lexeme::close_paren) {
        if (read) parser.consume(Lexeme::comma);
        read = true;
        builder.keep(Exp(parser));
    }
    parser.consume(Lexeme::close_paren);
    return builder.ExpListRule();
}

template <typename istream>
AST::ptr<AST::Exp> _Exp(Parser<istream>& parser,
                        AST::ptr<AST::Exp>&& lhs) {
    Builder<AST::ptr<AST::Exp>> builder(parser.make_id());
    builder.keep(std::move(lhs));
    switch (Lexeme(parser[0])) {
    case Lexeme::and_operator:
        parser.consume(Lexeme::and_operator);
        return _Exp(parser, builder.keep(Exp(parser)).andExp());
    case Lexeme::less_operator:
        parser.consume(Lexeme::less_operator);
        return _Exp(parser, builder.keep(Exp(parser)).lessExp());
    case Lexeme::plus_operator:
        parser.consume(Lexeme::plus_operator);
        return _Exp(parser, builder.keep(Exp(parser)).sumExp());
    case Lexeme::minus_operator:
        parser.consume(Lexeme::minus_operator);
        return _Exp(parser, builder.keep(Exp(parser)).minusExp());
    case Lexeme::times_operator:
        parser.consume(Lexeme::times_operator);
        return _Exp(parser, builder.keep(Exp(parser)).prodExp());
    case Lexeme::open_bracket:
        parser.consume(Lexeme::open_bracket);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_bracket);
        return _Exp(parser, builder.indexingExp());
    case Lexeme::period:
        parser.consume(Lexeme::period);
        if (Lexeme(parser[0]) == Lexeme::lenght_keyword) {
            parser.consume(Lexeme::lenght_keyword);
            return _Exp(parser, builder.lengthExp());
        }
        return _Exp(parser,
                    builder.keep(parser.consume(Lexeme::identifier))
                        .keep(ExpList(parser))
                        .methodCallExp());
    default:
        return builder.lhs();
    }
}

template <typename istream>
AST::ptr<AST::Exp> Exp(Parser<istream>& parser) {
    Builder<AST::ptr<AST::Exp>> builder(parser.make_id());
    switch (Lexeme(parser[0])) {
    case Lexeme::integer_literal:
        return _Exp(parser, builder
                                .keep(std::stoi(parser.consume(
                                    Lexeme::integer_literal)))
                                .integerExp());
    case Lexeme::true_keyword:
        parser.consume(Lexeme::true_keyword);
        return _Exp(parser, builder.trueExp());
    case Lexeme::false_keyword:
        parser.consume(Lexeme::false_keyword);
        return _Exp(parser, builder.falseExp());
    case Lexeme::identifier:
        return _Exp(parser,
                    builder.keep(parser.consume(Lexeme::identifier))
                        .identifierExp());
    case Lexeme::this_keyword:
        parser.consume(Lexeme::this_keyword);
        return _Exp(parser, builder.thisExp());
    case Lexeme::new_keyword:
        parser.consume(Lexeme::new_keyword);
        if (Lexeme(parser[0]) == Lexeme::int_keyword) {
            parser.consume(Lexeme::int_keyword);
            parser.consume(Lexeme::open_bracket);
            builder.keep(Exp(parser));
            parser.consume(Lexeme::close_bracket);
            return _Exp(parser, builder.newArrayExp());
        }
        builder.keep(parser.consume(Lexeme::identifier));
        parser.consume(Lexeme::open_paren);
        parser.consume(Lexeme::close_paren);
        return _Exp(parser, builder.newObjectExp());
    case Lexeme::bang:
        parser.consume(Lexeme::bang);
        return _Exp(parser, builder.keep(Exp(parser)).bangExp());
    case Lexeme::open_paren:
        parser.consume(Lexeme::open_paren);
        builder.keep(Exp(parser));
        parser.consume(Lexeme::close_paren);
        return _Exp(parser, builder.parenExp());
    default:
        throw Unexpected{Lexeme(parser[0])};
    }
}

} // namespace Parser
