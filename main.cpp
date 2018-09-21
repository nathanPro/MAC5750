#include "error.h"
#include "parser.h"
#include "util.h"
#include <fstream>

class TranslationUnit {
    std::string filename;
    std::ifstream stream;
    ParserContext<std::ifstream> ctx;
    AST::ptr<AST::Program> syntax_tree;

  public:
    TranslationUnit(std::string name)
        : filename(name), stream(filename, std::ios::in), ctx(stream),
          syntax_tree(Parser::Program(ctx)) {}

    bool check() {
        Reporter rep(std::cout, ctx.errors);
        Grammar::visit(rep, *syntax_tree);
        return rep;
    }
};

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cerr << "Please input a file\n";
        return 0;
    }
    auto TU = TranslationUnit(argv[1]);
    if (TU.check()) std::cout << "File parsed successfully\n";
}
