#include "parser.h"
#include "util.h"

int main() {
    auto lexer = Lexer(std::cin, 2);
    try {
        auto r = Parser::Program(lexer);
    } catch (...) {}
    return 0;
}
