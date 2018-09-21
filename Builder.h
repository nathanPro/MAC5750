#ifndef BCC_ASTBUILDER
#define BCC_ASTBUILDER
#include "AST.h"
#include "lexer.h"

template <typename T> class ASTBuilder;

struct Unexpected {
    Lexeme lex;
};

struct Mismatch {
    Lexeme expected;
    Lexeme found;
};

struct WrongIdentifier {
    std::string expected;
    std::string found;
};

struct ParsingError {
    std::vector<std::string> ctx;
    std::variant<Unexpected, Mismatch, WrongIdentifier> inner;
};

using ASTErrorData = std::vector<std::unique_ptr<ParsingError>>;

template <typename istream> class ParserContext {
    Lexer<istream> tokens;
    std::vector<std::string> context;
    std::vector<int32_t> lines;
    int idx;

  public:
    std::vector<ASTErrorData> errors;
    ParserContext(istream& stream) : tokens(stream, 2), idx(0) {}
    LexState operator[](int i) { return tokens[i]; }
    friend class ASTBuilder<istream>;
};

template <typename istream> class ASTBuilder {
    ParserContext<istream>& parser;
    AST::Node id;
    AST::ptr<AST::MainClass> main;
    std::vector<std::string> W;
    std::vector<AST::ptr<AST::ClassDecl>> C;
    std::vector<AST::ptr<AST::VarDecl>> V;
    std::vector<AST::ptr<AST::MethodDecl>> M;
    std::vector<AST::ptr<AST::Type>> T;
    std::vector<AST::ptr<AST::Exp>> E;
    std::vector<AST::ptr<AST::Stm>> S;
    AST::ptr<AST::FormalList> list;
    AST::ptr<AST::ExpList> arguments;
    int32_t value;
    bool pop = false;

  public:
    ASTBuilder(ParserContext<istream>& __parser)
        : parser(__parser), id(parser.idx++) {
        parser.errors.push_back({});
    }

    ASTBuilder(ParserContext<istream>& __parser, std::string label)
        : parser(__parser), id(parser.idx++) {
        parser.errors.push_back({});
        parser.context.push_back(label);
        parser.lines.push_back(parser[0].third);
        pop = true;
    }

    ~ASTBuilder() {
        if (pop) {
            parser.context.pop_back();
            parser.lines.pop_back();
        }
    }

    ASTBuilder& operator<<(Lexeme lex) {
        if (Lexeme(parser[0]) != lex) {
            auto err   = std::make_unique<ParsingError>();
            err->inner = Mismatch{lex, Lexeme(parser[0])};
            err->ctx   = parser.context;
            parser.errors[id.get()].push_back(std::move(err));
            while (Lexeme(parser[0]) != Lexeme::eof &&
                   Lexeme(parser[0]) != lex)
                ++parser.tokens;
        }
        if (Lexeme::identifier == lex)
            W.push_back(parser[0].second);
        else if (Lexeme::integer_literal == lex)
            value = std::stoi(parser[0].second);
        ++parser.tokens;
        return *this;
    }

    ASTBuilder& operator<<(std::string in) {
        if (parser[0].second != in) {
            auto err   = std::make_unique<ParsingError>();
            err->inner = WrongIdentifier{in, parser[0].second};
            err->ctx   = parser.context;
            parser.errors[id.get()].push_back(std::move(err));
            while (Lexeme(parser[0]) != Lexeme::eof &&
                   !(Lexeme(parser[0]) == Lexeme::identifier &&
                     parser[0].second == in))
                ++parser.tokens;
        }
        ++parser.tokens;
        return *this;
    }

    void unexpected(Lexeme un) {
        if (un == Lexeme::eof) return;
        auto err   = std::make_unique<ParsingError>();
        err->inner = Unexpected{un};
        err->ctx   = parser.context;
        parser.errors[id.get()].push_back(std::move(err));
        ++parser.tokens;
    }

    ASTBuilder& operator<<(AST::ptr<AST::MainClass>&& in) {
        main.reset(in.release());
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::ClassDecl>&& in) {
        C.push_back(std::move(in));
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::VarDecl>&& in) {
        V.push_back(std::move(in));
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::MethodDecl>&& in) {
        M.push_back(std::move(in));
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::Type>&& in) {
        T.push_back(std::move(in));
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::Exp>&& in) {
        E.push_back(std::move(in));
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::Stm>&& in) {
        S.push_back(std::move(in));
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::ExpList>&& in) {
        arguments.reset(in.release());
        return *this;
    }

    ASTBuilder& operator<<(AST::ptr<AST::FormalList>&& in) {
        list.reset(in.release());
        return *this;
    }

    AST::ptr<AST::Program> ProgramRule() {
        return std::make_unique<AST::Program>(
            AST::ProgramRule{id, std::move(main), std::move(C)});
    }

    AST::ptr<AST::MainClass> MainClassRule() {
        return std::make_unique<AST::MainClass>(
            AST::MainClassRule{id, W[0], W[1], std::move(S[0])});
    }

    AST::ptr<AST::ClassDecl> ClassDeclNoInheritance() {
        return std::make_unique<AST::ClassDecl>(
            AST::ClassDeclNoInheritance{id, W[0], std::move(V),
                                        std::move(M)});
    }

    AST::ptr<AST::ClassDecl> ClassDeclInheritance() {
        return std::make_unique<AST::ClassDecl>(
            AST::ClassDeclInheritance{id, W[0], W[1], std::move(V),
                                      std::move(M)});
    }

    AST::ptr<AST::MethodDecl> MethodDeclRule() {
        return std::make_unique<AST::MethodDecl>(AST::MethodDeclRule{
            id, std::move(T[0]), W[0], std::move(list), std::move(V),
            std::move(S), std::move(E[0])});
    }

    AST::ptr<AST::VarDecl> VarDeclRule() {
        return std::make_unique<AST::VarDecl>(
            AST::VarDeclRule{id, std::move(T[0]), W[0]});
    }

    AST::ptr<AST::FormalList> FormalListRule() {
        std::vector<AST::FormalDecl> D;
        int s = T.size() < W.size() ? T.size() : W.size();
        for (int i = 0; i < s; i++)
            D.push_back(AST::FormalDecl{std::move(T[i]), W[i]});

        return std::make_unique<AST::FormalList>(
            AST::FormalListRule{id, std::move(D)});
    }

    AST::ptr<AST::Type> integerArrayType() {
        return std::make_unique<AST::Type>(AST::integerArrayType{id});
    }

    AST::ptr<AST::Type> booleanType() {
        return std::make_unique<AST::Type>(AST::booleanType{id});
    }

    AST::ptr<AST::Type> integerType() {
        return std::make_unique<AST::Type>(AST::integerType{id});
    }

    AST::ptr<AST::Type> classType() {
        return std::make_unique<AST::Type>(AST::classType{id, W[0]});
    }

    AST::ptr<AST::ExpList> ExpListRule() {
        return std::make_unique<AST::ExpList>(
            AST::ExpListRule{id, std::move(E)});
    }

    AST::ptr<AST::Stm> blockStm() {
        return std::make_unique<AST::Stm>(
            AST::blockStm{id, std::move(S)});
    }

    AST::ptr<AST::Stm> ifStm() {
        return std::make_unique<AST::Stm>(AST::ifStm{
            id, std::move(E[0]), std::move(S[0]), std::move(S[1])});
    }

    AST::ptr<AST::Stm> whileStm() {
        return std::make_unique<AST::Stm>(
            AST::whileStm{id, std::move(E[0]), std::move(S[0])});
    }

    AST::ptr<AST::Stm> printStm() {
        return std::make_unique<AST::Stm>(
            AST::printStm{id, std::move(E[0])});
    }

    AST::ptr<AST::Stm> assignStm() {
        return std::make_unique<AST::Stm>(
            AST::assignStm{id, W[0], std::move(E[0])});
    }

    AST::ptr<AST::Stm> indexAssignStm() {
        return std::make_unique<AST::Stm>(AST::indexAssignStm{
            id, W[0], std::move(E[0]), std::move(E[1])});
    }

    AST::ptr<AST::Exp> lhs() { return std::move(E[0]); }

    AST::ptr<AST::Exp> andExp() {
        return std::make_unique<AST::Exp>(
            AST::andExp{id, std::move(E[0]), std::move(E[1])});
    }

    AST::ptr<AST::Exp> lessExp() {
        return std::make_unique<AST::Exp>(
            AST::lessExp{id, std::move(E[0]), std::move(E[1])});
    }

    AST::ptr<AST::Exp> sumExp() {
        return std::make_unique<AST::Exp>(
            AST::sumExp{id, std::move(E[0]), std::move(E[1])});
    }

    AST::ptr<AST::Exp> minusExp() {
        return std::make_unique<AST::Exp>(
            AST::minusExp{id, std::move(E[0]), std::move(E[1])});
    }

    AST::ptr<AST::Exp> prodExp() {
        return std::make_unique<AST::Exp>(
            AST::lessExp{id, std::move(E[0]), std::move(E[1])});
    }

    AST::ptr<AST::Exp> indexingExp() {
        return std::make_unique<AST::Exp>(
            AST::indexingExp{id, std::move(E[0]), std::move(E[1])});
    }

    AST::ptr<AST::Exp> lengthExp() {
        return std::make_unique<AST::Exp>(
            AST::lengthExp{id, std::move(E[0])});
    }

    AST::ptr<AST::Exp> methodCallExp() {
        return std::make_unique<AST::Exp>(AST::methodCallExp{
            id, std::move(E[0]), W[0], std::move(arguments)});
    }

    AST::ptr<AST::Exp> integerExp() {
        return std::make_unique<AST::Exp>(AST::integerExp{id, value});
    }

    AST::ptr<AST::Exp> trueExp() {
        return std::make_unique<AST::Exp>(AST::trueExp{id});
    }

    AST::ptr<AST::Exp> falseExp() {
        return std::make_unique<AST::Exp>(AST::falseExp{id});
    }

    AST::ptr<AST::Exp> thisExp() {
        return std::make_unique<AST::Exp>(AST::thisExp{id});
    }

    AST::ptr<AST::Exp> identifierExp() {
        return std::make_unique<AST::Exp>(
            AST::identifierExp{id, W[0]});
    }

    AST::ptr<AST::Exp> newArrayExp() {
        return std::make_unique<AST::Exp>(
            AST::newArrayExp{id, std::move(E[0])});
    }

    AST::ptr<AST::Exp> newObjectExp() {
        return std::make_unique<AST::Exp>(
            AST::newObjectExp{id, W[0]});
    }

    AST::ptr<AST::Exp> bangExp() {
        return std::make_unique<AST::Exp>(
            AST::bangExp{id, std::move(E[0])});
    }
    AST::ptr<AST::Exp> parenExp() {
        return std::make_unique<AST::Exp>(
            AST::parenExp{id, std::move(E[0])});
    }
};
#endif
