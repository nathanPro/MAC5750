#include "parser.h"
#include "util.h"

int main() {
    auto parser = Parser::Parser(std::cin);
    try {
        auto r = Parser::Program(parser);
    } catch (...) {}
    return 0;
}
