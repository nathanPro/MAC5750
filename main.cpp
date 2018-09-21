#include "parser.h"
#include "util.h"

int main() {
    auto ctx = ParserContext(std::cin);
    auto r   = Parser::Program(ctx);
}
