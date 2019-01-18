#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree& _tree)
    : out(_out), tree(_tree), need(tree), rg(1)
{
    flatten(3);

    *out << prelude;
    for (auto const& mtd : tree.methods) generate_fragment(mtd);
    tree.dump(*out);
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
        su_codegen(tree.get_mem(ref).exp, k);
        emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(1);
            return pop.build();
        }());
        tree.get_mem(ref).exp = tree.get_register(1);
        emit([&] {
            IRBuilder move(tree);
            move << IR::IRTag::MOVE << tree.get_register(2) << ref;
            return move.build();
        }());
        emit([&] {
            IRBuilder push(tree);
            push << IR::IRTag::PUSH << tree.get_register(2);
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
            pop << IR::IRTag::POP << tree.get_register(2);
            return pop.build();
        }());
        emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(1);
            return pop.build();
        }());
        lhs = tree.get_register(1);
        rhs = tree.get_register(2);

        if (tree.get_type(ref) == IR::IRTag::MOVE)
            tree.get_move(ref) = IR::Move{lhs, rhs};
        else
            tree.get_binop(ref) =
                IR::Binop{tree.get_binop(ref).op, lhs, rhs};

        emit(ref);

        if (tree.get_type(ref) == IR::IRTag::BINOP)
            emit([&] {
                IRBuilder push(tree);
                push << IR::IRTag::PUSH << tree.get_register(1);
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

void codegen::emit(int inst)
{
    if (tree.stm_seq.size() && tree.stm_seq.back() != inst)
        tree.stm_seq.push_back(inst);
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

void codegen::flatten(int k)
{
    tree.fix_registers(k);
    for (auto& mtd : tree.methods) {
        tree.stm_seq = {};
        for (int s : mtd.second.stms) __flat(s, k);
        mtd.second.stms = std::move(tree.stm_seq);
    }
}
} // namespace GEN
