#include "parser.h"
#include "util.h"

int main() {
    auto parser = Parser::Parser(std::cin);
    auto r      = Parser::Program(parser);
}
