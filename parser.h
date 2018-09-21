#ifndef BCC_PARSER
#define BCC_PARSER
#include "AST.h"
#include "Builder.h"
#include "util.h"

namespace Parser {

template <typename istream>
AST::ptr<AST::Exp> Exp(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::ExpList> ExpList(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::Stm> Stm(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::Type> Type(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::FormalList> FormalList(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::VarDecl> VarDecl(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::MethodDecl> MethodDecl(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::ClassDecl> ClassDecl(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::MainClass> MainClass(ParserContext<istream>&);
template <typename istream>
AST::ptr<AST::Program> Program(ParserContext<istream>&);

template <typename istream>
AST::ptr<AST::Program> Program(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    builder << MainClass(parser);
    while (Lexeme(parser[0]) != Lexeme::eof)
        builder << ClassDecl(parser);
    return builder.ProgramRule();
}

template <typename istream>
AST::ptr<AST::MainClass> MainClass(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    builder << Lexeme::class_keyword << Lexeme::identifier
            << Lexeme::open_brace << Lexeme::public_keyword
            << Lexeme::static_keyword << Lexeme::void_keyword
            << std::string("main") << Lexeme::open_paren
            << Lexeme::string_keyword << Lexeme::open_bracket
            << Lexeme::close_bracket << Lexeme::identifier
            << Lexeme::close_paren << Lexeme::open_brace
            << Stm(parser) << Lexeme::close_brace
            << Lexeme::close_brace;
    return builder.MainClassRule();
}

template <typename istream>
AST::ptr<AST::ClassDecl> ClassDecl(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    builder << Lexeme::class_keyword << Lexeme::identifier;

    bool has_superclass =
        (Lexeme(parser[0]) == Lexeme::extends_keyword);

    if (has_superclass)
        builder << Lexeme::extends_keyword << Lexeme::identifier;

    builder << Lexeme::open_brace;
    while (Lexeme(parser[0]) != Lexeme::close_brace &&
           Lexeme(parser[0]) != Lexeme::public_keyword)
        builder << VarDecl(parser);

    while (Lexeme(parser[0]) != Lexeme::close_brace)
        builder << MethodDecl(parser);
    builder << Lexeme::close_brace;

    if (has_superclass)
        return builder.ClassDeclInheritance();
    else
        return builder.ClassDeclNoInheritance();
}

template <typename istream>
AST::ptr<AST::MethodDecl> MethodDecl(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    builder << Lexeme::public_keyword << Type(parser)
            << Lexeme::identifier << FormalList(parser)
            << Lexeme::open_brace;

    while (Lexeme(parser[0]) == Lexeme::boolean_keyword ||
           Lexeme(parser[0]) == Lexeme::int_keyword ||
           (Lexeme(parser[0]) == Lexeme::identifier &&
            Lexeme(parser[1]) == Lexeme::identifier))
        builder << VarDecl(parser);

    while (Lexeme(parser[0]) != Lexeme::return_keyword)
        builder << Stm(parser);
    builder << Lexeme::return_keyword << Exp(parser)
            << Lexeme::semicolon << Lexeme::close_brace;
    return builder.MethodDeclRule();
}

template <typename istream>
AST::ptr<AST::VarDecl> VarDecl(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    builder << Type(parser) << Lexeme::identifier
            << Lexeme::semicolon;
    return builder.VarDeclRule();
}

template <typename istream>
AST::ptr<AST::FormalList> FormalList(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    builder << Lexeme::open_paren;
    bool first = true;
    while (Lexeme(parser[0]) != Lexeme::close_paren) {
        if (!first) builder << Lexeme::comma;
        first = false;
        builder << Type(parser) << Lexeme::identifier;
    }
    builder << Lexeme::close_paren;
    return builder.FormalListRule();
}

template <typename istream>
AST::ptr<AST::Type> Type(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    switch (Lexeme(parser[0])) {
    case Lexeme::boolean_keyword:
        builder << Lexeme::boolean_keyword;
        return builder.booleanType();
    case Lexeme::identifier:
        builder << Lexeme::identifier;
        return builder.classType();
    case Lexeme::int_keyword:
        builder << Lexeme::int_keyword;
        if (Lexeme(parser[0]) == Lexeme::open_bracket) {
            builder << Lexeme::open_bracket << Lexeme::close_bracket;
            return builder.integerArrayType();
        }
        return builder.integerType();
    default:
        builder.unexpected(Lexeme(parser[0]));
        return builder.integerType();
    }
}

template <typename istream>
AST::ptr<AST::Stm> Stm(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    switch (Lexeme(parser[0])) {
    case Lexeme::open_brace:
        builder << Lexeme::open_brace;
        while (Lexeme(parser[0]) != Lexeme::close_brace)
            builder << Stm(parser);
        builder << Lexeme::close_brace;
        return builder.blockStm();
    case Lexeme::if_keyword:
        builder << Lexeme::if_keyword << Lexeme::open_paren
                << Exp(parser) << Lexeme::close_paren << Stm(parser)
                << Lexeme::else_keyword << Stm(parser);
        return builder.ifStm();
    case Lexeme::while_keyword:
        builder << Lexeme::while_keyword << Lexeme::open_paren
                << Exp(parser) << Lexeme::close_paren << Stm(parser);
        return builder.whileStm();
    case Lexeme::println_keyword:
        builder << Lexeme::println_keyword << Lexeme::open_paren
                << Exp(parser) << Lexeme::close_paren
                << Lexeme::semicolon;
        return builder.printStm();
    case Lexeme::identifier:
        builder << Lexeme::identifier;
        if (Lexeme(parser[0]) == Lexeme::equals_sign) {
            builder << Lexeme::equals_sign << Exp(parser)
                    << Lexeme::semicolon;
            return builder.assignStm();
        }
        builder << Lexeme::open_bracket << Exp(parser)
                << Lexeme::close_bracket << Lexeme::equals_sign
                << Exp(parser) << Lexeme::semicolon;
        return builder.indexAssignStm();
    default:
        builder.unexpected(Lexeme(parser[0]));
        return builder.blockStm();
    }
}

template <typename istream>
AST::ptr<AST::ExpList> ExpList(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    builder << Lexeme::open_paren;
    bool first = true;
    while (Lexeme(parser[0]) != Lexeme::close_paren) {
        if (!first) builder << Lexeme::comma;
        first = false;
        builder << Exp(parser);
    }
    builder << Lexeme::close_paren;
    return builder.ExpListRule();
}

template <typename istream>
AST::ptr<AST::Exp> _Exp(ParserContext<istream>& parser,
                        AST::ptr<AST::Exp>&& lhs) {
    ASTBuilder builder(parser);
    builder << std::move(lhs);
    switch (Lexeme(parser[0])) {
    case Lexeme::and_operator:
        builder << Lexeme::and_operator << Exp(parser);
        return _Exp(parser, builder.andExp());
    case Lexeme::less_operator:
        builder << Lexeme::less_operator << Exp(parser);
        return _Exp(parser, builder.lessExp());
    case Lexeme::plus_operator:
        builder << Lexeme::plus_operator << Exp(parser);
        return _Exp(parser, builder.sumExp());
    case Lexeme::minus_operator:
        builder << Lexeme::minus_operator << Exp(parser);
        return _Exp(parser, builder.minusExp());
    case Lexeme::times_operator:
        builder << Lexeme::times_operator << Exp(parser);
        return _Exp(parser, builder.prodExp());
    case Lexeme::open_bracket:
        builder << Lexeme::open_bracket << Exp(parser)
                << Lexeme::close_bracket;
        return _Exp(parser, builder.indexingExp());
    case Lexeme::period:
        builder << Lexeme::period;
        if (Lexeme(parser[0]) == Lexeme::lenght_keyword) {
            builder << Lexeme::lenght_keyword;
            return _Exp(parser, builder.lengthExp());
        }
        builder << Lexeme::identifier << ExpList(parser);
        return _Exp(parser, builder.methodCallExp());
    default:
        return builder.lhs();
    }
}

template <typename istream>
AST::ptr<AST::Exp> Exp(ParserContext<istream>& parser) {
    ASTBuilder builder(parser);
    switch (Lexeme(parser[0])) {
    case Lexeme::integer_literal:
        builder << Lexeme::integer_literal;
        return _Exp(parser, builder.integerExp());
    case Lexeme::true_keyword:
        builder << Lexeme::true_keyword;
        return _Exp(parser, builder.trueExp());
    case Lexeme::false_keyword:
        builder << Lexeme::false_keyword;
        return _Exp(parser, builder.falseExp());
    case Lexeme::identifier:
        builder << Lexeme::identifier;
        return _Exp(parser, builder.identifierExp());
    case Lexeme::this_keyword:
        builder << Lexeme::this_keyword;
        return _Exp(parser, builder.thisExp());
    case Lexeme::new_keyword:
        builder << Lexeme::new_keyword;
        if (Lexeme(parser[0]) == Lexeme::int_keyword) {
            builder << Lexeme::int_keyword << Lexeme::open_bracket
                    << Exp(parser) << Lexeme::close_bracket;
            return _Exp(parser, builder.newArrayExp());
        }
        builder << Lexeme::identifier << Lexeme::open_paren
                << Lexeme::close_paren;
        return _Exp(parser, builder.newObjectExp());
    case Lexeme::bang:
        builder << Lexeme::bang << Exp(parser);
        return _Exp(parser, builder.bangExp());
    case Lexeme::open_paren:
        builder << Lexeme::open_paren << Exp(parser)
                << Lexeme::close_paren;
        return _Exp(parser, builder.parenExp());
    default:
        builder.unexpected(Lexeme(parser[0]));
        return builder.parenExp();
    }
}

} // namespace Parser
#endif
