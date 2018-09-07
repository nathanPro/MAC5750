#include "parser.h"

int main() {
    auto lexer = Lexer(std::cin);
    auto r     = Parser::Exp(lexer);
}
