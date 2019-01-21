#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree& _tree)
    : out(_out), tree(_tree), need(tree), rg(1)
{
    tree.simplify();
    flatten(9);
    prepare_x86_call();

    Util::write(*out, prelude);
    for (auto const& mtd : tree.methods) generate_fragment(mtd);
}

void codegen::__flat_rec(int ref)
{
    int lhs, rhs;

    switch (tree.get_type(ref)) {
    case IR::IRTag::TEMP:
    case IR::IRTag::PUSH:
    case IR::IRTag::POP:
    case IR::IRTag::JMP:
    case IR::IRTag::LABEL:
        break;

    case IR::IRTag::EXP:
        __flat_rec(tree.get_exp(ref).exp);
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
    case IR::IRTag::EXP:
        __flat_rec(ref);
        break;

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
    auto const& [name, frag] = mtd;
    if (tree.aliases.count(name))
        for (auto const& alias : tree.aliases.at(name))
            Util::write(*out, alias, ":\n");

    Util::write(*out, name, ":\n");
    IR::Catamorphism<x86Output, std::string> F(tree);
    for (int s : frag.stms)
        if (tree.get_type(s) == IR::IRTag::LABEL)
            Util::write(*out, F(s), ":");
        else
            Util::write(*out, F(s));
    Util::write(*out, "ret");
}

void codegen::flatten(int k)
{
    tree.fix_registers(k);
    for (auto& mtd : tree.methods) {
        tree.stm_seq = {};
        for (int s : mtd.second.stms) __flat(s);
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
    tree.emit([&] {
        IRBuilder push(tree);
        push << IR::IRTag::PUSH << tree.get_register(7);
        return push.build();
    }());
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

void codegen::__align_x86_call()
{
    for (auto& mtd : tree.methods) {
        tree.stm_seq = {};
        int parity   = 0;
        for (int s : mtd.second.stms) {
            if (tree.get_type(s) == IR::IRTag::CALL && parity) {
                int cte = [&] {
                    IRBuilder c(tree);
                    c << IR::IRTag::CONST << 8;
                    return c.build();
                }();
                tree.emit([&] {
                    IRBuilder binop(tree);
                    binop << IR::IRTag::BINOP << IR::BinopId::PLUS
                          << tree.get_register(0) << cte;
                    return binop.build();
                }());
                tree.emit(s);
                tree.emit([&] {
                    IRBuilder binop(tree);
                    binop << IR::IRTag::BINOP << IR::BinopId::MINUS
                          << tree.get_register(0) << cte;
                    return binop.build();
                }());
            } else {
                tree.emit(s);
                if (tree.get_type(s) == IR::IRTag::PUSH ||
                    tree.get_type(s) == IR::IRTag::POP)
                    parity ^= 1;
            }
        }
        mtd.second.stms = std::move(tree.stm_seq);
    }
}

void codegen::prepare_x86_call()
{
    for (auto& mtd : tree.methods) {
        auto& [name, frag] = mtd;

        int _disp = frag.stack.spill_size;
        if (_disp % 16) _disp += 8;

        auto disp = [&] {
            IRBuilder c(tree);
            c << IR::IRTag::CONST << _disp;
            return c.build();
        }();

        tree.stm_seq = {};
        {
            tree.emit([&] {
                IRBuilder push(tree);
                push << IR::IRTag::PUSH << tree.get_register(0);
                return push.build();
            }());
            tree.emit([&] {
                IRBuilder binop(tree);
                binop << IR::IRTag::BINOP << IR::BinopId::MINUS
                      << tree.get_register(8) << disp;
                return binop.build();
            }());
            tree.emit([&] {
                IRBuilder move(tree);
                move << IR::IRTag::MOVE << tree.get_register(0)
                     << tree.get_register(8);
                return move.build();
            }());
        }
        {
            std::vector<int> args;
            args.push_back(frag.stack.tp);
            for (auto& it : frag.stack.arguments)
                args.push_back(it.second);
            std::sort(begin(args), end(args));
            int rhs = tree.get_register(1) - 1;
            for (int arg : args) {
                if (rhs < tree.get_register(6))
                    rhs++;
                else if (rhs == tree.get_register(6)) {
                    int cte = [&] {
                        IRBuilder c(tree);
                        c << IR::IRTag::CONST << (_disp + 16);
                        return c.build();
                    }();
                    int binop = [&] {
                        IRBuilder binop(tree);
                        binop << IR::IRTag::BINOP << IR::BinopId::PLUS
                              << tree.get_register(0) << cte;
                        return binop.build();
                    }();
                    rhs = [&] {
                        IRBuilder mem(tree);
                        mem << IR::IRTag::MEM << binop;
                        return mem.build();
                    }();
                } else
                    tree.get_const(
                            tree.get_binop(tree.get_mem(rhs).exp).rhs)
                        .value += 8;
                int aux = [&] {
                    IRBuilder move(tree);
                    move << IR::IRTag::MOVE << arg << rhs;
                    return move.build();
                }();
                tree.stm_seq.pop_back();
                __flat(aux);
            }
        }
        for (int s : frag.stms) {
            if (tree.get_type(s) != IR::IRTag::CALL)
                tree.emit(s);
            else
                __x86_call(s);
        }

        if (name != std::string("main")) {
            tree.emit([&] {
                IRBuilder pop(tree);
                pop << IR::IRTag::POP << tree.get_register(7);
                return pop.build();
            }());
        }
        tree.emit([&] {
            IRBuilder add(tree);
            add << IR::IRTag::BINOP << IR::BinopId::PLUS
                << tree.get_register(0) << disp;
            return add.build();
        }());
        tree.emit([&] {
            IRBuilder move(tree);
            move << IR::IRTag::MOVE << tree.get_register(8)
                 << tree.get_register(0);
            return move.build();
        }());
        tree.emit([&] {
            IRBuilder pop(tree);
            pop << IR::IRTag::POP << tree.get_register(0);
            return pop.build();
        }());
        frag.stms = std::move(tree.stm_seq);
    }
    __align_x86_call();
}
} // namespace GEN
