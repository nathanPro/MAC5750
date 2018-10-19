#ifndef BCC_LOGGER
#define BCC_LOGGER
#include "AST.h"

class Logger {
    std::vector<int32_t>         lines;
    std::vector<std::string>     context;
    std::vector<AST::ErrorData>& errors;

  public:
    Logger(std::vector<AST::ErrorData>&);
    void push(std::string label, int line);
    void pop();
    void mismatch(Lexeme expected, Lexeme found, AST::Node id);
    void mismatch(std::string expected, std::string found,
                  AST::Node id);
    void unexpected(Lexeme un, AST::Node id);
};
#endif
