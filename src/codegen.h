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

    void __x86_call(int);
    void __align_x86_call();
    void __flat_rec(int);
    void __flat(int);
    void emit(int);

    int rg;

  public:
    codegen(std::ostream*, IR::Tree&);
    void generate_fragment(fragment_t);
    void flatten(int k);
    void prepare_x86_call();
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

template <typename C> struct x86Output {

    std::string operator()(IR::Const const& c)
    {
        return std::to_string(c.value);
    }
    std::string operator()(IR::Reg const& r)
    {
        static std::vector<std::string> regs = {"rbp", "rdi", "rsi",
                                                "rdx", "rcx", "r8",
                                                "r9",  "rax", "rsp"};
        return regs[r.id];
    }
    std::string operator()(IR::Temp const& t)
    {
        return std::string("TEMP{") + std::to_string(t.id) +
               std::string("}");
    }
    std::string operator()(IR::Binop const& b)
    {
        static std::vector<std::string> names = {
            "add", "sub", "imul", "/",       "and",
            "or",  "<<",  ">>",   "ARSHIFT", "xor"};
        return names[b.op] + std::string(" ") + fmap(b.lhs) +
               std::string(", ") + fmap(b.rhs);
    }
    std::string operator()(IR::Mem const& m)
    {
        return std::string("[") + fmap(m.exp) + std::string("]");
    }
    std::string operator()(IR::Call const& c)
    {
        return std::string("call ") + c.fn;
    }
    std::string operator()(IR::Cmp const& c)
    {
        return std::string("cmp ") + fmap(c.lhs) + std::string(", ") +
               fmap(c.rhs) + std::string("\npushfq\npop rdi");
    }
    std::string operator()(IR::Move const& m)
    {
        return std::string("mov ") + fmap(m.dst) + std::string(", ") +
               fmap(m.src);
    }
    std::string operator()(IR::Exp const& e)
    {
        return std::string("EXP{") + fmap(e.exp) + std::string("}");
    }
    std::string operator()(IR::Jmp const& j)
    {
        return std::string("jmp ") + std::to_string(j.target);
    }
    std::string operator()(IR::Cjmp const& c)
    {
        return std::string("test ") + fmap(c.temp) +
               std::string(", ") + fmap(c.temp) +
               std::string("\njne ") + fmap(c.target);
    }
    std::string operator()(IR::Label const& l)
    {
        return std::string("L") + std::to_string(l.id);
    }
    std::string operator()(IR::Push const& p)
    {
        return std::string("push ") + fmap(p.ref);
    }
    std::string operator()(IR::Pop const& p)
    {
        return std::string("pop ") + fmap(p.ref);
    }

    x86Output(C&& __fmap) : fmap(__fmap) {}
    C fmap;
};
} // namespace GEN

#endif
