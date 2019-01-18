#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree& _tree)
    : out(_out), tree(_tree), need(tree), rg(1)
{
    flatten(0);

    *out << prelude;
    for (auto const& mtd : tree.methods) generate_fragment(mtd);
}

void codegen::su_codegen(int ref, int k)
{
    int lhs, rhs;

    switch (tree.get_type(ref)) {
    case IR::IRTag::TEMP:
    case IR::IRTag::CALL:
    case IR::IRTag::CMP:
    case IR::IRTag::PUSH:
    case IR::IRTag::POP:
    case IR::IRTag::EXP:
    case IR::IRTag::JMP:
    case IR::IRTag::LABEL:
    case IR::IRTag::CJMP:
        Util::write(std::cerr, "wut", ref,
                    static_cast<int>(tree.get_type(ref)));
        Util::write(std::cerr, tree);
        throw;
        break;

    case IR::IRTag::REG:
    case IR::IRTag::CONST:
        emit([&] {
            IRBuilder push(tree);
            push << IR::IRTag::PUSH << ref;
            return push.build();
        }());
        break;

    case IR::IRTag::MEM: {
        emit([&] {
            IRBuilder pop(tree);
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 1;
            pop << IR::IRTag::POP << reg.build();
            return pop.build();
        }());
        tree.get_mem(ref).exp = [&] {
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 1;
            return reg.build();
        }();
        emit([&] {
            IRBuilder move(tree);
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 2;
            move << IR::IRTag::MOVE << reg.build() << ref;
            return move.build();
        }());
        emit([&] {
            IRBuilder push(tree);
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 2;
            push << IR::IRTag::PUSH << reg.build();
            return push.build();
        }());
    } break;

    case IR::IRTag::MOVE:
    case IR::IRTag::BINOP: {
        if (tree.get_type(ref) == IR::IRTag::MOVE)
            lhs = tree.get_move(ref).src,
            rhs = tree.get_move(ref).dst;
        else
            lhs = tree.get_binop(ref).lhs,
            rhs = tree.get_binop(ref).rhs;

        su_codegen(lhs, k);
        su_codegen(rhs, k);
        emit([&] {
            IRBuilder pop(tree);
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 2;
            pop << IR::IRTag::POP << reg.build();
            return pop.build();
        }());
        emit([&] {
            IRBuilder pop(tree);
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 1;
            pop << IR::IRTag::POP << reg.build();
            return pop.build();
        }());
        lhs = [&] {
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 1;
            return reg.build();
        }();
        rhs = [&] {
            IRBuilder reg(tree);
            reg << IR::IRTag::REG << 2;
            return reg.build();
        }();

        if (tree.get_type(ref) == IR::IRTag::MOVE)
            tree.get_move(ref) = IR::Move{lhs, rhs};
        else
            tree.get_binop(ref) =
                IR::Binop{tree.get_binop(ref).op, lhs, rhs};

        emit(ref);

        if (tree.get_type(ref) == IR::IRTag::BINOP)
            emit([&] {
                IRBuilder push(tree);
                IRBuilder reg(tree);
                reg << IR::IRTag::REG << 1;
                push << IR::IRTag::PUSH << reg.build();
                return push.build();
            }());
        break;
    }
    }
}

void codegen::__flat(int ref, int k)
{
    switch (tree.get_type(ref)) {

    case IR::IRTag::MOVE: {
        IR::Move mv = tree.get_move(ref);
        if (tree.get_type(mv.src) == IR::IRTag::CALL ||
            tree.get_type(mv.src) == IR::IRTag::CMP)
            emit(ref);
        else
            su_codegen(ref, k);
    } break;

    case IR::IRTag::EXP:
    case IR::IRTag::JMP:
    case IR::IRTag::LABEL:
    case IR::IRTag::CJMP:
        emit(ref);
        break;

    case IR::IRTag::CONST:
    case IR::IRTag::REG:
    case IR::IRTag::TEMP:
    case IR::IRTag::BINOP:
    case IR::IRTag::MEM:
    case IR::IRTag::CALL:
    case IR::IRTag::CMP:
    case IR::IRTag::PUSH:
    case IR::IRTag::POP:
        emit(-1);
        break;
    }
}

void codegen::emit(int inst) { tree.stm_seq.push_back(inst); }

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

void codegen::flatten(int k)
{
    for (auto& mtd : tree.methods) {
        tree.stm_seq = {};
        for (int s : mtd.second.stms) __flat(s, k);
        mtd.second.stms = std::move(tree.stm_seq);
    }
}
} // namespace GEN
