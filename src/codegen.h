#ifndef BCC_CODEGEN
#define BCC_CODEGEN

#include "IR.h"
#include "IRBuilder.h"
#include "helper.h"
#include <algorithm>
#include <ostream>
#include <string>

namespace GEN
{

template <typename C> struct SethiUllman {
    int operator()(IR::Temp const&) { return 1; }
    int operator()(IR::Const const&) { return 0; }
    int handle_binary_node(int lhs, int rhs)
    {
        lhs = fmap(lhs);
        rhs = fmap(rhs);
        if (lhs == rhs) return 1 + lhs;
        return std::max(lhs, rhs);
    }
    int operator()(IR::Move const& move)
    {
        return handle_binary_node(move.dst, move.src);
    }
    int operator()(IR::Binop const& binop)
    {
        return handle_binary_node(binop.lhs, binop.rhs);
    }
    int operator()(IR::Mem const& mem)
    {
        return std::max(1, fmap(mem.exp));
    }
    int operator()(IR::Exp const& exp) { return fmap(exp.exp); }
    template <typename T> int operator()(T const&) { return 0; }
    SethiUllman(C&& __fmap) : fmap(__fmap) {}
    C fmap;
};

class codegen
{
    using fragment_t =
        typename std::map<std::string, IR::fragment>::value_type;
    std::ostream*                      out;
    IR::Tree&                          tree;
    IR::Catamorphism<SethiUllman, int> need;

    void __flat_rec(int);
    void __flat(int);
    void emit(int);

    int rg;

  public:
    codegen(std::ostream*, IR::Tree&);
    void generate_fragment(fragment_t);
    void flatten(int k);
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
