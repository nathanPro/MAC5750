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

    {
        IR::Tree cp = tree;
        Util::write(std::cerr, "Base Tree");
        cp.dump(std::cerr);

        cp.simplify();
        Util::write(std::cerr, "Simplified Tree");
        cp.dump(std::cerr);
    }

    std::ofstream out(argv[2]);
    GEN::codegen  code(&out, tree);

    {
        Util::write(std::cerr, "Final Tree");
        tree.dump(std::cerr);
    }

    code.output();
}
