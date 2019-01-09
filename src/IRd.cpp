#include "helper.h"
#include "parser.h"
#include "translate.h"
#include "util.h"

int main(int argc, char** argv)
{
    if (argc == 1) {
        Util::write(std::cerr, "Please give an input file");
        return 1;
    }

    TranslationUnit tu(std::string{argv[1]});
    IR::Tree        tree;
    translate(tree, tu.syntax_tree);
    Util::write(std::cout, tree);
}
