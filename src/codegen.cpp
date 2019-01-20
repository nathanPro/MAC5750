#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree& _tree)
    : out(_out), tree(_tree), need(tree), rg(1)
{
    tree.simplify();
    flatten(3);

    // *out << prelude;
    // for (auto const& mtd : tree.methods) generate_fragment(mtd);
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
        tree.emit([&] {
            IRBuilder push(tree);
            push << IR::IRTag::PUSH << ref;
            return push.build();
        }());
        break;

    case IR::IRTag::MEM: {
        su_codegen(tree.get_mem(ref).exp, k);
        tree.emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(1);
            return pop.build();
        }());
        ref = [&] {
            IRBuilder mem(tree);
            mem << IR::IRTag::MEM << tree.get_register(1);
            return mem.build();
        }();
        tree.emit([&] {
            IRBuilder move(tree);
            move << IR::IRTag::MOVE << tree.get_register(2) << ref;
            return move.build();
        }());
        tree.emit([&] {
            IRBuilder push(tree);
            push << IR::IRTag::PUSH << tree.get_register(2);
            return push.build();
        }());
    } break;

    case IR::IRTag::MOVE:
    case IR::IRTag::BINOP: {
        if (tree.get_type(ref) == IR::IRTag::MOVE)
            lhs = tree.get_move(ref).dst,
            rhs = tree.get_move(ref).src;
        else
            lhs = tree.get_binop(ref).lhs,
            rhs = tree.get_binop(ref).rhs;

        su_codegen(lhs, k);
        su_codegen(rhs, k);
        tree.emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(2);
            return pop.build();
        }());
        tree.emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(1);
            return pop.build();
        }());
        lhs = tree.get_register(1);
        rhs = tree.get_register(2);

        if (tree.get_type(ref) == IR::IRTag::MOVE)
            tree.emit([&] {
                IRBuilder move(tree);
                move << IR::IRTag::MOVE << lhs << rhs;
                return move.build();
            }());
        else
            tree.emit([&] {
                IRBuilder binop(tree);
                binop << IR::IRTag::BINOP << tree.get_binop(ref).op
                      << lhs << rhs;
                return binop.build();
            }());

        if (tree.get_type(ref) == IR::IRTag::BINOP)
            tree.emit([&] {
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
            tree.emit(ref);
        else
            su_codegen(ref, k);
    } break;

    case IR::IRTag::EXP:
    case IR::IRTag::JMP:
    case IR::IRTag::LABEL:
    case IR::IRTag::CJMP:
        tree.emit(ref);
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
        tree.emit(-1);
        break;
    }
}

void codegen::generate_fragment(fragment_t mtd)
{
    // auto const& [name, frag] = mtd;
    // if (tree.aliases.count(name))
    //     for (auto const& alias : tree.aliases.at(name))
    //         *out << alias << ":\n";

    // *out << name << ":\n";
    // IR::Catamorphism<SethiUllman, int> G(tree);
    // for (int i : frag.stms) *out << i << " " << G(i) << "\n";
    // *out << "\n; C'EST FINI\n";
}

void codegen::flatten(int k)
{
    tree.fix_registers(k);
    for (auto& mtd : tree.methods) {
        tree.stm_seq = {};
        for (int s : mtd.second.stms) __flat(s, k);
        if (mtd.first != std::string("main")) {
            {
                auto ret = tree.stm_seq.back();
                tree.stm_seq.pop_back();
                su_codegen(tree.get_exp(ret).exp, k);
            }
            tree.emit([&] {
                IRBuilder pop(tree);
                pop << IR::IRTag::POP << tree.get_register(1);
                return pop.build();
            }());
            tree.emit([&] {
                IRBuilder ret(tree);
                ret << IR::IRTag::EXP << tree.get_register(1);
                return ret.build();
            }());
        }
        mtd.second.stms = std::move(tree.stm_seq);
    }
}
} // namespace GEN
