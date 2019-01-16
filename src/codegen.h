#ifndef BCC_CODEGEN
#define BCC_CODEGEN

#include "IR.h"
#include "helper.h"
#include <ostream>
#include <string>

namespace GEN
{
class codegen
{
    std::ostream*   out;
    IR::Tree const& tree;

  public:
    codegen(std::ostream*, IR::Tree const&);
};

// clang-format off
inline std::string prelude = {"global main" "\n"
                              "extern printf" "\n"
                              "extern malloc" "\n"
                              "extern memcp" "\n"
                              "section .dat" "\n"
                              "format: db \"%d\", 10, 0" "\n"
                              "section .text" "\n"
                              "print:" "\n"
                              "    enter 0, 0" "\n"
                              "    mov rsi, rdi" "\n"
                              "    mov rdi, format" "\n"
                              "    call printf" "\n"
                              "    leave" "\n"
                              "    ret" "\n"};
// clang-format on
} // namespace GEN

#endif
