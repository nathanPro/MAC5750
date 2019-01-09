#include "helper.h"
#include "parser.h"
#include "translate.h"
#include "util.h"

int main()
{
    TranslationUnit tu("../input/calc.miniJava");
    IR::Tree        tree;
    translate(tree, tu.syntax_tree);
    Util::write(std::cout, tree);
}
