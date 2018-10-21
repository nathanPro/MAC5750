#ifndef BCC_PARSER
#define BCC_PARSER
#include "AST.h"
#include "Builder.h"
#include "logger.h"
#include "util.h"
#include <fstream>

template <typename istream> class Parser {
    AST::Exp _Exp(AST::Exp&& lhs) {
        using std::move;
        AST::Builder builder(*this);
        builder << std::move(lhs);
        switch (Lexeme(tokens[0])) {
        case Lexeme::and_operator:
            builder << Lexeme::and_operator << Exp();
            return _Exp(AST::andExp(move(builder)));
        case Lexeme::less_operator:
            builder << Lexeme::less_operator << Exp();
            return _Exp(AST::lessExp(move(builder)));
        case Lexeme::plus_operator:
            builder << Lexeme::plus_operator << Exp();
            return _Exp(AST::sumExp(move(builder)));
        case Lexeme::minus_operator:
            builder << Lexeme::minus_operator << Exp();
            return _Exp(AST::minusExp(move(builder)));
        case Lexeme::times_operator:
            builder << Lexeme::times_operator << Exp();
            return _Exp(AST::prodExp(move(builder)));
        case Lexeme::open_bracket:
            builder << Lexeme::open_bracket << Exp()
                    << Lexeme::close_bracket;
            return _Exp(AST::indexingExp(move(builder)));
        case Lexeme::period:
            builder << Lexeme::period;
            if (Lexeme(tokens[0]) == Lexeme::lenght_keyword) {
                builder << Lexeme::lenght_keyword;
                return _Exp(AST::lengthExp(move(builder)));
            }
            builder << Lexeme::identifier << ExpList();
            return _Exp(AST::methodCallExp(move(builder)));
        default:
            return builder.lhs();
        }
    }

    void record_context(std::string label) {
        logger.push(label, tokens[0].third);
    }

    void drop_context() { logger.pop(); }

    void mismatch(Lexeme lex, AST::Node id) {
        logger.mismatch(lex, Lexeme(tokens[0]), id);
        while (Lexeme(tokens[0]) != Lexeme::eof &&
               Lexeme(tokens[0]) != lex)
            ++tokens;
    }

    void mismatch(std::string in, AST::Node id) {
        logger.mismatch(in, tokens[0].second, id);
        while (Lexeme(tokens[0]) != Lexeme::eof &&
               !(Lexeme(tokens[0]) == Lexeme::identifier &&
                 tokens[0].second == in))
            ++tokens;
    }

    void unexpected(Lexeme un, AST::Node id) {
        logger.unexpected(un, id);
        ++tokens;
    }

  public:
    Parser(istream& stream)
        : tokens(stream, 2), idx(0), logger(errors) {}
    LexState operator[](int i) { return tokens[i]; }
    friend class AST::Builder<istream>;

    AST::Exp Exp() {
        using std::move;
        AST::Builder builder(*this, "Expression");
        switch (Lexeme(tokens[0])) {
        case Lexeme::integer_literal:
            builder << Lexeme::integer_literal;
            return _Exp(AST::integerExp(move(builder)));
        case Lexeme::true_keyword:
            builder << Lexeme::true_keyword;
            return _Exp(AST::trueExp(move(builder)));
        case Lexeme::false_keyword:
            builder << Lexeme::false_keyword;
            return _Exp(AST::falseExp(move(builder)));
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            return _Exp(AST::identifierExp(move(builder)));
        case Lexeme::this_keyword:
            builder << Lexeme::this_keyword;
            return _Exp(AST::thisExp(move(builder)));
        case Lexeme::new_keyword:
            builder << Lexeme::new_keyword;
            if (Lexeme(tokens[0]) == Lexeme::int_keyword) {
                builder << Lexeme::int_keyword << Lexeme::open_bracket
                        << Exp() << Lexeme::close_bracket;
                return _Exp(AST::newArrayExp(move(builder)));
            }
            builder << Lexeme::identifier << Lexeme::open_paren
                    << Lexeme::close_paren;
            return _Exp(AST::newObjectExp(move(builder)));
        case Lexeme::bang:
            builder << Lexeme::bang << Exp();
            return _Exp(AST::bangExp(move(builder)));
        case Lexeme::open_paren:
            builder << Lexeme::open_paren << Exp()
                    << Lexeme::close_paren;
            return _Exp(AST::parenExp(move(builder)));
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return AST::falseExp(move(builder));
        }
    }

    AST::ExpList ExpList() {
        AST::Builder builder(*this, "Method Call List");
        builder << Lexeme::open_paren;
        bool first = true;
        while (Lexeme(tokens[0]) != Lexeme::close_paren) {
            if (!first) builder << Lexeme::comma;
            first = false;
            builder << Exp();
        }
        builder << Lexeme::close_paren;
        return AST::ExpListRule(std::move(builder));
    }

    AST::Stm Stm() {
        using std::move;
        AST::Builder builder(*this, "Statement");
        switch (Lexeme(tokens[0])) {
        case Lexeme::open_brace:
            builder << Lexeme::open_brace;
            while (Lexeme(tokens[0]) != Lexeme::close_brace)
                builder << Stm();
            builder << Lexeme::close_brace;
            return AST::blockStm(move(builder));
        case Lexeme::if_keyword:
            builder << Lexeme::if_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren << Stm()
                    << Lexeme::else_keyword << Stm();
            return AST::ifStm(move(builder));
        case Lexeme::while_keyword:
            builder << Lexeme::while_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren << Stm();
            return AST::whileStm(move(builder));
        case Lexeme::println_keyword:
            builder << Lexeme::println_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren
                    << Lexeme::semicolon;
            return AST::printStm(move(builder));
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            if (Lexeme(tokens[0]) == Lexeme::equals_sign) {
                builder << Lexeme::equals_sign << Exp()
                        << Lexeme::semicolon;
                return AST::assignStm(move(builder));
            }
            builder << Lexeme::open_bracket << Exp()
                    << Lexeme::close_bracket << Lexeme::equals_sign
                    << Exp() << Lexeme::semicolon;
            return AST::indexAssignStm(move(builder));
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return AST::blockStm(move(builder));
        }
    }

    AST::Type Type() {
        AST::Builder builder(*this, "Type");
        switch (Lexeme(tokens[0])) {
        case Lexeme::boolean_keyword:
            builder << Lexeme::boolean_keyword;
            return AST::booleanType(std::move(builder));
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            return AST::classType(std::move(builder));
        case Lexeme::int_keyword:
            builder << Lexeme::int_keyword;
            if (Lexeme(tokens[0]) == Lexeme::open_bracket) {
                builder << Lexeme::open_bracket
                        << Lexeme::close_bracket;
                return AST::integerArrayType(std::move(builder));
            }
            return AST::integerType(std::move(builder));
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return AST::integerType(std::move(builder));
        }
    }

    AST::FormalList FormalList() {
        AST::Builder builder(*this, "Method Argument List");
        builder << Lexeme::open_paren;
        bool first = true;
        while (Lexeme(tokens[0]) != Lexeme::close_paren) {
            if (!first) builder << Lexeme::comma;
            first = false;
            builder << Type() << Lexeme::identifier;
        }
        builder << Lexeme::close_paren;
        return AST::FormalListRule(std::move(builder));
    }

    AST::VarDecl VarDecl() {
        AST::Builder builder(*this, "Variable Declaration");
        builder << Type() << Lexeme::identifier << Lexeme::semicolon;
        return AST::VarDeclRule(std::move(builder));
    }

    AST::MethodDecl MethodDecl() {
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
        return AST::MethodDeclRule(std::move(builder));
    }

    AST::ClassDecl ClassDecl() {
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
            return AST::ClassDeclInheritance(std::move(builder));
        else
            return AST::ClassDeclNoInheritance(std::move(builder));
    }

    AST::MainClass MainClass() {
        AST::Builder builder(*this, "Main Class");
        builder << Lexeme::class_keyword << Lexeme::identifier
                << Lexeme::open_brace << Lexeme::public_keyword
                << Lexeme::static_keyword << Lexeme::void_keyword
                << std::string("main") << Lexeme::open_paren
                << Lexeme::string_keyword << Lexeme::open_bracket
                << Lexeme::close_bracket << Lexeme::identifier
                << Lexeme::close_paren << Lexeme::open_brace << Stm()
                << Lexeme::close_brace << Lexeme::close_brace;
        return AST::MainClassRule(std::move(builder));
    }

    AST::Program Program() {
        AST::Builder builder(*this);
        builder << MainClass();
        while (Lexeme(tokens[0]) != Lexeme::eof)
            builder << ClassDecl();
        return AST::ProgramRule(std::move(builder));
    }

  public:
    std::vector<AST::ErrorData> errors;

  private:
    Lexer<istream> tokens;
    int            idx;
    Logger         logger;
};

class TranslationUnit {
    std::string           filename;
    std::ifstream         stream;
    Parser<std::ifstream> parser;
    AST::Program          syntax_tree;

  public:
    TranslationUnit(std::string name)
        : filename(name), stream(filename, std::ios::in),
          parser(stream), syntax_tree(parser.Program()) {}

    bool check() {
        Reporter rep(std::cout, parser.errors);
        Grammar::visit(rep, syntax_tree);
        return rep;
    }
};
#endif
