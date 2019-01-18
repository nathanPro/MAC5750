#include "codegen.h"
#include "helper.h"
#include "parser.h"
#include "translate.h"
#include "util.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        Util::write(std::cerr, "Please give an input file");
        return 1;
    }
    if (argc < 3) {
        Util::write(std::cerr, "Please give an output file");
        return 1;
    }

    TranslationUnit tu(std::string{argv[1]});
    IR::Tree        tree;
    translate(tree, tu.syntax_tree);
    tree.simplify();
    GEN::codegen(&std::cout, tree);
    Util::write(std::cout, tree);
    /*
    Util::write(std::cout, "\n\n\tGENERATED ASSEMBLY\n");
    */
}
