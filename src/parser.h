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
        using std::move;
        AST::Builder builder(*this);
        builder << std::move(lhs);
        switch (Lexeme(tokens[0])) {
        case Lexeme::and_operator:
            builder << Lexeme::and_operator << Exp();
            return _Exp(AST::andExp::build(move(builder)));
        case Lexeme::less_operator:
            builder << Lexeme::less_operator << Exp();
            return _Exp(AST::lessExp::build(move(builder)));
        case Lexeme::plus_operator:
            builder << Lexeme::plus_operator << Exp();
            return _Exp(AST::sumExp::build(move(builder)));
        case Lexeme::minus_operator:
            builder << Lexeme::minus_operator << Exp();
            return _Exp(AST::minusExp::build(move(builder)));
        case Lexeme::times_operator:
            builder << Lexeme::times_operator << Exp();
            return _Exp(AST::prodExp::build(move(builder)));
        case Lexeme::open_bracket:
            builder << Lexeme::open_bracket << Exp()
                    << Lexeme::close_bracket;
            return _Exp(AST::indexingExp::build(move(builder)));
        case Lexeme::period:
            builder << Lexeme::period;
            if (Lexeme(tokens[0]) == Lexeme::lenght_keyword) {
                builder << Lexeme::lenght_keyword;
                return _Exp(AST::lengthExp::build(move(builder)));
            }
            builder << Lexeme::identifier << ExpList();
            return _Exp(AST::methodCallExp::build(move(builder)));
        default:
            return builder.lhs();
        }
    }

    void record_context(std::string label) {
        context.push_back(label);
        lines.push_back(tokens[0].third);
    }

    void drop_context() {
        context.pop_back();
        lines.pop_back();
    }

    void mismatch(Lexeme lex, AST::Node id) {
        auto err   = std::make_unique<AST::ParsingError>();
        err->inner = AST::Mismatch{Lexeme(tokens[0]), lex};
        err->ctx   = context;
        errors[id.get()].push_back(std::move(err));
        while (Lexeme(tokens[0]) != Lexeme::eof &&
               Lexeme(tokens[0]) != lex)
            ++tokens;
    }

    void mismatch(std::string in, AST::Node id) {
        auto err   = std::make_unique<AST::ParsingError>();
        err->inner = AST::WrongIdentifier{in, tokens[0].second};
        err->ctx   = context;
        errors[id.get()].push_back(std::move(err));
        while (Lexeme(tokens[0]) != Lexeme::eof &&
               !(Lexeme(tokens[0]) == Lexeme::identifier &&
                 tokens[0].second == in))
            ++tokens;
    }

    void unexpected(Lexeme un, AST::Node id) {
        if (un == Lexeme::eof) return;
        auto err   = std::make_unique<AST::ParsingError>();
        err->inner = AST::Unexpected{un};
        err->ctx   = context;
        errors[id.get()].push_back(std::move(err));
        ++tokens;
    }

  public:
    std::vector<AST::ErrorData> errors;
    Parser(istream& stream) : tokens(stream, 2), idx(0) {}
    LexState operator[](int i) { return tokens[i]; }
    friend class AST::Builder<istream>;

    AST::ptr<AST::Exp> Exp() {
        using std::move;
        AST::Builder builder(*this, "Expression");
        switch (Lexeme(tokens[0])) {
        case Lexeme::integer_literal:
            builder << Lexeme::integer_literal;
            return _Exp(AST::integerExp::build(move(builder)));
        case Lexeme::true_keyword:
            builder << Lexeme::true_keyword;
            return _Exp(AST::trueExp::build(move(builder)));
        case Lexeme::false_keyword:
            builder << Lexeme::false_keyword;
            return _Exp(AST::falseExp::build(move(builder)));
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            return _Exp(AST::identifierExp::build(move(builder)));
        case Lexeme::this_keyword:
            builder << Lexeme::this_keyword;
            return _Exp(AST::thisExp::build(move(builder)));
        case Lexeme::new_keyword:
            builder << Lexeme::new_keyword;
            if (Lexeme(tokens[0]) == Lexeme::int_keyword) {
                builder << Lexeme::int_keyword << Lexeme::open_bracket
                        << Exp() << Lexeme::close_bracket;
                return _Exp(AST::newArrayExp::build(move(builder)));
            }
            builder << Lexeme::identifier << Lexeme::open_paren
                    << Lexeme::close_paren;
            return _Exp(AST::newObjectExp::build(move(builder)));
        case Lexeme::bang:
            builder << Lexeme::bang << Exp();
            return _Exp(AST::bangExp::build(move(builder)));
        case Lexeme::open_paren:
            builder << Lexeme::open_paren << Exp()
                    << Lexeme::close_paren;
            return _Exp(AST::parenExp::build(move(builder)));
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return AST::falseExp::build(move(builder));
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
        return AST::ExpListRule::build(std::move(builder));
    }

    AST::ptr<AST::Stm> Stm() {
        using std::move;
        AST::Builder builder(*this, "Statement");
        switch (Lexeme(tokens[0])) {
        case Lexeme::open_brace:
            builder << Lexeme::open_brace;
            while (Lexeme(tokens[0]) != Lexeme::close_brace)
                builder << Stm();
            builder << Lexeme::close_brace;
            return AST::blockStm::build(move(builder));
        case Lexeme::if_keyword:
            builder << Lexeme::if_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren << Stm()
                    << Lexeme::else_keyword << Stm();
            return AST::ifStm::build(move(builder));
        case Lexeme::while_keyword:
            builder << Lexeme::while_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren << Stm();
            return AST::whileStm::build(move(builder));
        case Lexeme::println_keyword:
            builder << Lexeme::println_keyword << Lexeme::open_paren
                    << Exp() << Lexeme::close_paren
                    << Lexeme::semicolon;
            return AST::printStm::build(move(builder));
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            if (Lexeme(tokens[0]) == Lexeme::equals_sign) {
                builder << Lexeme::equals_sign << Exp()
                        << Lexeme::semicolon;
                return AST::assignStm::build(move(builder));
            }
            builder << Lexeme::open_bracket << Exp()
                    << Lexeme::close_bracket << Lexeme::equals_sign
                    << Exp() << Lexeme::semicolon;
            return AST::indexAssignStm::build(move(builder));
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return AST::blockStm::build(move(builder));
        }
    }

    AST::ptr<AST::Type> Type() {
        AST::Builder builder(*this, "Type");
        switch (Lexeme(tokens[0])) {
        case Lexeme::boolean_keyword:
            builder << Lexeme::boolean_keyword;
            return AST::booleanType::build(std::move(builder));
        case Lexeme::identifier:
            builder << Lexeme::identifier;
            return AST::classType::build(std::move(builder));
        case Lexeme::int_keyword:
            builder << Lexeme::int_keyword;
            if (Lexeme(tokens[0]) == Lexeme::open_bracket) {
                builder << Lexeme::open_bracket
                        << Lexeme::close_bracket;
                return AST::integerArrayType::build(
                    std::move(builder));
            }
            return AST::integerType::build(std::move(builder));
        default:
            builder.unexpected(Lexeme(tokens[0]));
            return AST::integerType::build(std::move(builder));
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
        return AST::FormalListRule::build(std::move(builder));
    }

    AST::ptr<AST::VarDecl> VarDecl() {
        AST::Builder builder(*this, "Variable Declaration");
        builder << Type() << Lexeme::identifier << Lexeme::semicolon;
        return AST::VarDeclRule::build(std::move(builder));
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
        return AST::MethodDeclRule::build(std::move(builder));
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
            return AST::ClassDeclInheritance::build(
                std::move(builder));
        else
            return AST::ClassDeclNoInheritance::build(
                std::move(builder));
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
        return AST::MainClassRule::build(std::move(builder));
    }

    AST::ptr<AST::Program> Program() {
        AST::Builder builder(*this);
        builder << MainClass();
        while (Lexeme(tokens[0]) != Lexeme::eof)
            builder << ClassDecl();
        return AST::ProgramRule::build(std::move(builder));
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
