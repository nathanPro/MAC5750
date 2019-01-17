#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree const& _tree)
    : out(_out), tree(_tree)
{
    *out << prelude;

    for (auto const& mtd : tree.methods) generate_fragment(mtd);
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
} // namespace GEN
