#include "parser.h"
#include "util.h"

int main() {
    auto lexer = Lexer(std::cin, 2);
    try {
        auto r = Parser::Program(lexer);
    } catch (Parser::UnexpectedEnd e) {
        write(std::cerr, "BCC Error: File ended abruptly");
        return 1;
    } catch (Parser::Unexpected e) {
        write(std::cerr, "BCC Error: Unexpected lexeme", e.lex,
              "found");
        return 1;
    } catch (Parser::MismatchError e) {
        write(std::cerr, "BCC Error: Mismatch: Expected", e.expected,
              "found", e.found);
        return 1;
    }
    return 0;
}
