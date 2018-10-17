#ifndef BCC_PARSER
#define BCC_PARSER
#include "AST.h"
#include "Builder.h"
#include "util.h"
#include <fstream>

template <typename istream> class Parser {
    Lexer<istream>           tokens;
    std::vector<std::string> context;
    std::vector<int32_t>     lines;
    int                      idx;

    AST::ptr<AST::Exp> _Exp(AST::ptr<AST::Exp>&& lhs) {
        AST::Builder builder(*this);
        builder << std::move(lhs);
        switch (Lexeme(tokens[0])) {
        case Lexeme::and_operator:
            builder << Lexeme::and_operator << Exp();
            return _Exp(builder.andExp());
        case Lexeme::less_operator:
            builder << Lexeme::less_operator << Exp();
            return _Exp(builder.lessExp());
        case Lexeme::plus_operator:
            builder << Lexeme::plus_operator << Exp();
            return _Exp(builder.sumExp());
        case Lexeme::minus_operator:
            builder << Lexeme::minus_operator << Exp();
            return _Exp(builder.minusExp());
        case Lexeme::times_operator:
            builder << Lexeme::times_operator << Exp();
            return _Exp(builder.prodExp());
        case Lexeme::open_bracket:
            builder << Lexeme::open_bracket << Exp()
                    << Lexeme::close_bracket;
            return _Exp(builder.indexingExp());
        case Lexeme::period:
            builder << Lexeme::period;
            if (Lexeme(tokens[0]) == Lexeme::lenght_keyword) {
                builder << Lexeme::lenght_keyword;
                return _Exp(builder.lengthExp());
            }
            builder << Lexeme::identifier << ExpList();
            return _Exp(builder.methodCallExp());
        default:
            return builder.lhs();
        }
    }

  public:
    std::vector<AST::ErrorData> errors;
    Parser(istream& stream) : tokens(stream, 2), idx(0) {}
    LexState operator[](int i) { return tokens[i]; }
    friend class AST::Builder<istream>;

    AST::ptr<AST::Exp> Exp() {
        AST::Builder builder(*this, "Expression");
        switch (Lexeme(tokens[0])) {
        case Lexeme::integer_literal:
            builder << Lexeme::integer_literal;
            return _Exp(builder.integerExp());
        case Lexeme::true_keyword:
            builder << Lexeme::true_keyword;
            return _Exp(builder.trueExp());
        case Lexeme::false_keyword:
            builder << Lexeme::false_keyword;
            return _Exp(builder.falseExp());
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            return _Exp(builder.identifierExp());
        case Lexeme::this_keyword:
            builder << Lexeme::this_keyword;
            return _Exp(builder.thisExp());
        case Lexeme::new_keyword:
            builder << Lexeme::new_keyword;
            if (Lexeme(tokens[0]) == Lexeme::int_keyword) {
                builder << Lexeme::int_keyword << Lexeme::open_bracket
                        << Exp() << Lexeme::close_bracket;
                return _Exp(builder.newArrayExp());
            }
            builder << Lexeme::identifier << Lexeme::open_paren
                    << Lexeme::close_paren;
            return _Exp(builder.newObjectExp());
        case Lexeme::bang:
            builder << Lexeme::bang << Exp();
            return _Exp(builder.bangExp());
        case Lexeme::open_paren:
            builder << Lexeme::open_paren << Exp()
                    << Lexeme::close_paren;
            return _Exp(builder.parenExp());
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return builder.falseExp();
        }
    }

    AST::ptr<AST::ExpList> ExpList() {
        AST::Builder builder(*this, "Method Call List");
        builder << Lexeme::open_paren;
        bool first = true;
        while (Lexeme(tokens[0]) != Lexeme::close_paren) {
            if (!first) builder << Lexeme::comma;
            first = false;
            builder << Exp();
        }
        builder << Lexeme::close_paren;
        return builder.ExpListRule();
    }

    AST::ptr<AST::Stm> Stm() {
        AST::Builder builder(*this, "Statement");
        switch (Lexeme(tokens[0])) {
        case Lexeme::open_brace:
            builder << Lexeme::open_brace;
            while (Lexeme(tokens[0]) != Lexeme::close_brace)
                builder << Stm();
            builder << Lexeme::close_brace;
            return builder.blockStm();
        case Lexeme::if_keyword:
            builder << Lexeme::if_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren << Stm()
                    << Lexeme::else_keyword << Stm();
            return builder.ifStm();
        case Lexeme::while_keyword:
            builder << Lexeme::while_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren << Stm();
            return builder.whileStm();
        case Lexeme::println_keyword:
            builder << Lexeme::println_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren
                    << Lexeme::semicolon;
            return builder.printStm();
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            if (Lexeme(tokens[0]) == Lexeme::equals_sign) {
                builder << Lexeme::equals_sign << Exp()
                        << Lexeme::semicolon;
                return builder.assignStm();
            }
            builder << Lexeme::open_bracket << Exp()
                    << Lexeme::close_bracket << Lexeme::equals_sign
                    << Exp() << Lexeme::semicolon;
            return builder.indexAssignStm();
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return builder.blockStm();
        }
    }

    AST::ptr<AST::Type> Type() {
        AST::Builder builder(*this, "Type");
        switch (Lexeme(tokens[0])) {
        case Lexeme::boolean_keyword:
            builder << Lexeme::boolean_keyword;
            return builder.booleanType();
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            return builder.classType();
        case Lexeme::int_keyword:
            builder << Lexeme::int_keyword;
            if (Lexeme(tokens[0]) == Lexeme::open_bracket) {
                builder << Lexeme::open_bracket
                        << Lexeme::close_bracket;
                return builder.integerArrayType();
            }
            return builder.integerType();
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return builder.integerType();
        }
    }

    AST::ptr<AST::FormalList> FormalList() {
        AST::Builder builder(*this, "Method Argument List");
        builder << Lexeme::open_paren;
        bool first = true;
        while (Lexeme(tokens[0]) != Lexeme::close_paren) {
            if (!first) builder << Lexeme::comma;
            first = false;
            builder << Type() << Lexeme::identifier;
        }
        builder << Lexeme::close_paren;
        return builder.FormalListRule();
    }

    AST::ptr<AST::VarDecl> VarDecl() {
        AST::Builder builder(*this, "Variable Declaration");
        builder << Type() << Lexeme::identifier << Lexeme::semicolon;
        return builder.VarDeclRule();
    }

    AST::ptr<AST::MethodDecl> MethodDecl() {
        AST::Builder builder(*this, "Method Declaration");
        builder << Lexeme::public_keyword << Type()
                << Lexeme::identifier << FormalList()
                << Lexeme::open_brace;

        while (Lexeme(tokens[0]) == Lexeme::boolean_keyword ||
               Lexeme(tokens[0]) == Lexeme::int_keyword ||
               (Lexeme(tokens[0]) == Lexeme::identifier &&
                Lexeme(tokens[1]) == Lexeme::identifier))
            builder << VarDecl();

        while (Lexeme(tokens[0]) != Lexeme::return_keyword &&
               Lexeme(tokens[0]) != Lexeme::eof)
            builder << Stm();
        builder << Lexeme::return_keyword << Exp()
                << Lexeme::semicolon << Lexeme::close_brace;
        return builder.MethodDeclRule();
    }

    AST::ptr<AST::ClassDecl> ClassDecl() {
        AST::Builder builder(*this, "Class Declaration");
        builder << Lexeme::class_keyword << Lexeme::identifier;

        bool has_superclass =
            (Lexeme(tokens[0]) == Lexeme::extends_keyword);

        if (has_superclass)
            builder << Lexeme::extends_keyword << Lexeme::identifier;

        builder << Lexeme::open_brace;
        while (Lexeme(tokens[0]) != Lexeme::close_brace &&
               Lexeme(tokens[0]) != Lexeme::public_keyword)
            builder << VarDecl();

        while (Lexeme(tokens[0]) != Lexeme::close_brace)
            builder << MethodDecl();
        builder << Lexeme::close_brace;

        if (has_superclass)
            return builder.ClassDeclInheritance();
        else
            return builder.ClassDeclNoInheritance();
    }

    AST::ptr<AST::MainClass> MainClass() {
        AST::Builder builder(*this, "Main Class");
        builder << Lexeme::class_keyword << Lexeme::identifier
                << Lexeme::open_brace << Lexeme::public_keyword
                << Lexeme::static_keyword << Lexeme::void_keyword
                << std::string("main") << Lexeme::open_paren
                << Lexeme::string_keyword << Lexeme::open_bracket
                << Lexeme::close_bracket << Lexeme::identifier
                << Lexeme::close_paren << Lexeme::open_brace << Stm()
                << Lexeme::close_brace << Lexeme::close_brace;
        return builder.MainClassRule();
    }

    AST::ptr<AST::Program> Program() {
        AST::Builder builder(*this);
        builder << MainClass();
        while (Lexeme(tokens[0]) != Lexeme::eof)
            builder << ClassDecl();
        return builder.ProgramRule();
    }
};

class TranslationUnit {
    std::string            filename;
    std::ifstream          stream;
    Parser<std::ifstream>  parser;
    AST::ptr<AST::Program> syntax_tree;

  public:
    TranslationUnit(std::string name)
        : filename(name), stream(filename, std::ios::in),
          parser(stream), syntax_tree(parser.Program()) {}

    bool check() {
        Reporter rep(std::cout, parser.errors);
        Grammar::visit(rep, *syntax_tree);
        return rep;
    }
};
#endif
