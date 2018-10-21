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
    void mismatch(Lexeme expected, Lexeme found, int id);
    void mismatch(std::string expected, std::string found, int id);
    void unexpected(Lexeme un, int id);
};
#endif
