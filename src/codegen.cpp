#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree& _tree)
    : out(_out), tree(_tree), need(tree), rg(1)
{
    tree.simplify();
    flatten(7);
    prepare_x86_call();

    // *out << prelude;
    // for (auto const& mtd : tree.methods) generate_fragment(mtd);
}

void codegen::__flat_rec(int ref)
{
    int lhs, rhs;

    switch (tree.get_type(ref)) {
    case IR::IRTag::TEMP:
    case IR::IRTag::PUSH:
    case IR::IRTag::POP:
    case IR::IRTag::EXP:
    case IR::IRTag::JMP:
    case IR::IRTag::LABEL:
        break;

    case IR::IRTag::CALL:
        tree.emit(ref);
        break;

    case IR::IRTag::CJMP:
        __flat_rec(tree.get_cjmp(ref).temp);
        tree.emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(1);
            return pop.build();
        }());
        tree.emit([&] {
            IRBuilder cjmp(tree);
            cjmp << IR::IRTag::CJMP << tree.get_register(1)
                 << tree.get_cjmp(ref).target;
            return cjmp.build();
        }());
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
        __flat_rec(tree.get_mem(ref).exp);
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
    case IR::IRTag::BINOP:
    case IR::IRTag::CMP: {
        if (tree.get_type(ref) == IR::IRTag::MOVE)
            lhs = tree.get_mem(tree.get_move(ref).dst).exp,
            rhs = tree.get_move(ref).src;
        else if (tree.get_type(ref) == IR::IRTag::BINOP)
            lhs = tree.get_binop(ref).lhs,
            rhs = tree.get_binop(ref).rhs;
        else
            lhs = tree.get_cmp(ref).lhs, rhs = tree.get_cmp(ref).rhs;

        __flat_rec(lhs);
        __flat_rec(rhs);
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
        if (tree.get_type(ref) == IR::IRTag::MOVE) {
            lhs = [&] {
                IRBuilder mem(tree);
                mem << IR::IRTag::MEM << lhs;
                return mem.build();
            }();
        }
        rhs = tree.get_register(2);

        if (tree.get_type(ref) == IR::IRTag::BINOP)
            tree.emit([&] {
                IRBuilder binop(tree);
                binop << IR::IRTag::BINOP << tree.get_binop(ref).op
                      << lhs << rhs;
                return binop.build();
            }());
        else
            tree.emit([&] {
                IRBuilder curr(tree);
                curr << tree.get_type(ref) << lhs << rhs;
                return curr.build();
            }());

        if (tree.get_type(ref) != IR::IRTag::MOVE)
            tree.emit([&] {
                IRBuilder push(tree);
                push << IR::IRTag::PUSH << tree.get_register(1);
                return push.build();
            }());
        break;
    }
    }
}

void codegen::__flat(int ref)
{
    switch (tree.get_type(ref)) {

    case IR::IRTag::MOVE:
    case IR::IRTag::CJMP:
        __flat_rec(ref);
        break;

    case IR::IRTag::EXP:
    case IR::IRTag::LABEL:
    case IR::IRTag::JMP:
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
        for (int s : mtd.second.stms) __flat(s);
        if (mtd.first != std::string("main")) {
            {
                auto ret = tree.stm_seq.back();
                tree.stm_seq.pop_back();
                __flat_rec(tree.get_exp(ret).exp);
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

void codegen::__x86_call(int ref)
{
    auto const& [fn, _es] = tree.get_call(ref);
    auto es               = tree.get_explist(_es);

    std::reverse(begin(es), end(es));
    for (int e : es) __flat_rec(e);
    for (int i = 0; i < std::min<int>(6, es.size()); i++)
        tree.emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(1 + i);
            return pop.build();
        }());
    tree.emit(ref);
    if (6 < es.size()) {
        int cte = [&] {
            IRBuilder cte(tree);
            cte << IR::IRTag::CONST
                << 8 * (static_cast<int>(es.size()) - 6);
            return cte.build();
        }();
        tree.emit([&] {
            IRBuilder binop(tree);
            binop << IR::IRTag::BINOP << IR::BinopId::PLUS
                  << tree.get_register(0) << cte;
            return binop.build();
        }());
    }
}

void codegen::prepare_x86_call()
{
    for (auto& mtd : tree.methods) {
        tree.stm_seq = {};
        for (int s : mtd.second.stms) {
            if (tree.get_type(s) != IR::IRTag::CALL)
                tree.emit(s);
            else
                __x86_call(s);
        }
        mtd.second.stms = std::move(tree.stm_seq);
    }
}
} // namespace GEN
