#include "error.h"
#include "parser.h"
#include "util.h"

int main() {
    auto ctx = ParserContext(std::cin);
    auto r   = Parser::Program(ctx);
    Grammar::visit(Reporter(std::cout, ctx.errors), *r);
}
