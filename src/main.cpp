#include "error.h"
#include "parser.h"
#include "util.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cerr << "Please input a file\n";
        return 0;
    }
    auto TU = TranslationUnit(argv[1]);
    if (TU.check()) std::cout << "File parsed successfully\n";
}
