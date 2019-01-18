#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree& _tree)
    : out(_out), tree(_tree), need(tree)
{
    flatten();

    *out << prelude;
    for (auto const& mtd : tree.methods) generate_fragment(mtd);
}

int codegen::su_codegen(int ref)
{
    auto mv = tree.get_move(ref);
    if (tree.get_type(mv.src) == IR::IRTag::CALL ||
        tree.get_type(mv.src) == IR::IRTag::CMP)
        return ref;
    return 0;
}

int codegen::__flat(int ref)
{
    switch (static_cast<IR::IRTag>(tree.get_type(ref))) {
    case IR::IRTag::CONST:
    case IR::IRTag::REG:
    case IR::IRTag::TEMP:
    case IR::IRTag::BINOP:
    case IR::IRTag::MEM:
    case IR::IRTag::CALL:
    case IR::IRTag::CMP:
    case IR::IRTag::PUSH:
    case IR::IRTag::POP:
        return -1;

    case IR::IRTag::MOVE:
        return su_codegen(ref);

    case IR::IRTag::EXP:
    case IR::IRTag::JMP:
    case IR::IRTag::LABEL:
    case IR::IRTag::CJMP:
        return ref;
    }
    __builtin_unreachable();
}

void codegen::generate_fragment(fragment_t mtd)
{
    auto const& [name, frag] = mtd;
    if (tree.aliases.count(name))
        for (auto const& alias : tree.aliases.at(name))
            *out << alias << ":\n";

    *out << name << ":\n";
    IR::Catamorphism<SethiUllman, int> G(tree);
    for (int i : frag.stms) *out << i << " " << G(i) << "\n";
    *out << "\n; C'EST FINI\n";
}

void codegen::flatten()
{
    for (auto& mtd : tree.methods) {
        tree.stm_seq = {};
        for (int s : mtd.second.stms)
            tree.stm_seq.push_back(__flat(s));
        mtd.second.stms = std::move(tree.stm_seq);
    }
}
} // namespace GEN
