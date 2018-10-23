#ifndef BCC_PARSER
#define BCC_PARSER
#include "AST.h"
#include "error.h"
#include "logger.h"
#include "util.h"
#include <fstream>

class Parser {
    AST::Exp _Exp(AST::Exp&& lhs);

    void record_context(std::string label);
    void drop_context();
    void mismatch(Lexeme lex, int id);
    void mismatch(std::string in, int id);
    void unexpected(Lexeme un, int id);

  public:
    Parser(std::istream* stream);
    LexState operator[](int i);
    friend class AST::Builder;

    AST::Exp        Exp();
    AST::ExpList    ExpList();
    AST::Stm        Stm();
    AST::Type       Type();
    AST::FormalList FormalList();
    AST::VarDecl    VarDecl();
    AST::MethodDecl MethodDecl();
    AST::ClassDecl  ClassDecl();
    AST::MainClass  MainClass();
    AST::Program    Program();

  public:
    std::vector<AST::ErrorData> errors;

  private:
    Lexer  tokens;
    int    idx;
    Logger logger;
};

class TranslationUnit {
    std::string   filename;
    std::ifstream stream;
    Parser        parser;

  public:
    AST::Program syntax_tree;

    TranslationUnit(std::string name);
    bool check();
};
#endif
