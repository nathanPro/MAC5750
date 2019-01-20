#ifndef BCC_IR
#define BCC_IR

#include "util.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

class IRBuilder;
namespace IR
{

struct BadAccess {
    int found;
    int expected;
};

enum RelopId { EQ, NE, LT, GT, LE, GE, ULT, ULE, UGT, UGE };

enum BinopId {
    PLUS,
    MINUS,
    MUL,
    DIV,
    AND,
    OR,
    LSHIFT,
    RSHIFT,
    ARSHIFT,
    XOR
};

enum class IRTag {
    CONST,
    REG,
    TEMP,
    BINOP,
    MEM,
    CALL,
    CMP,
    MOVE,
    EXP,
    JMP,
    LABEL,
    CJMP,
    PUSH,
    POP
};

int is_exp(IRTag tag);

struct Const {
    int value;
};

struct Reg {
    int id;
};

struct Temp {
    int id;
};

struct Binop {
    int op;
    int lhs;
    int rhs;
};

struct Mem {
    int exp;
};

struct Call {
    std::string fn;
    int         explist;
};

struct Move {
    int dst;
    int src;
};

struct Exp {
    int exp;
};

struct Jmp {
    int target;
};

struct Cjmp {
    int temp;
    int target;
};

struct Label {
    int id;
};

struct Cmp {
    int lhs;
    int rhs;
};

struct Push {
    int ref;
};

struct Pop {
    int ref;
};

using Explist = std::vector<int>;

struct fragmentGuard;

struct activation_record {
    std::map<std::string, int> arguments;
    int                        sp;
    int                        tp;
    int                        spill_size;
};

struct fragment {
    activation_record stack;
    std::vector<int>  stms;
    size_t            size() const;
};

class Tree;
class label_handle
{
    friend class Tree;
    int ref;
    label_handle(int);

  public:
    operator int() const;
};

class Tree
{
    friend class ::IRBuilder;
    friend struct fragmentGuard;
    friend std::ostream& operator<<(std::ostream&, Tree&);
    template <template <typename C> typename F, typename R>
    friend struct Catamorphism;
    template <typename FT>
    friend void tree_dump(std::ostream&, Tree const&, FT&);

    int tmp;
    int lbl;

    // Type checking info
    std::vector<int>     kind;
    std::vector<int>     pos;
    std::vector<Explist> _explist;

    // Exp types
    std::vector<Const> _const;
    std::vector<Reg>   _reg;
    std::vector<Temp>  _temp;
    std::vector<Binop> _binop;
    std::vector<Mem>   _mem;
    std::vector<Call>  _call;
    std::vector<Cmp>   _cmp;

    // Stm types
    std::vector<Move>  _move;
    std::vector<Exp>   _exp;
    std::vector<Jmp>   _jmp;
    std::vector<Label> _label;
    std::vector<Cjmp>  _cjmp;
    std::vector<Push>  _push;
    std::vector<Pop>   _pop;

    int base_register;

    void spill();
    void mark_sp();

  public:
    Tree();
    Const& get_const(int ref);
    Reg&   get_reg(int ref);
    Temp&  get_temp(int ref);
    Binop& get_binop(int ref);
    Mem&   get_mem(int ref);
    Call&  get_call(int ref);
    Cmp&   get_cmp(int ref);

    Move&  get_move(int ref);
    Exp&   get_exp(int ref);
    Jmp&   get_jmp(int ref);
    Label& get_label(int ref);
    Cjmp&  get_cjmp(int ref);
    Push&  get_push(int ref);
    Pop&   get_pop(int ref);

    Const const& get_const(int ref) const;
    Reg const&   get_reg(int ref) const;
    Temp const&  get_temp(int ref) const;
    Binop const& get_binop(int ref) const;
    Mem const&   get_mem(int ref) const;
    Call const&  get_call(int ref) const;
    Cmp const&   get_cmp(int ref) const;

    Move const&  get_move(int ref) const;
    Exp const&   get_exp(int ref) const;
    Jmp const&   get_jmp(int ref) const;
    Label const& get_label(int ref) const;
    Cjmp const&  get_cjmp(int ref) const;
    Push const&  get_push(int ref) const;
    Pop const&   get_pop(int ref) const;

    // generic functinality
    IRTag   get_type(int ref) const;
    size_t  size() const;
    Explist get_explist(int) const;
    int     keep_explist(Explist&&);
    int     new_temp();

    label_handle new_label();
    int          place_label(label_handle&&);

    void simplify();
    void fix_registers(int);
    int  get_register(int);

    void dump(std::ostream&) const;
    void emit(int);

    std::vector<int>                             stm_seq;
    std::map<std::string, fragment>              methods;
    std::map<std::string, std::set<std::string>> aliases;
};

std::ostream& operator<<(std::ostream& out, Tree const&);

template <template <typename C> typename F, typename R>
struct Catamorphism {
    using rec_t = std::function<R(int)>;
    Tree const&      tree;
    std::vector<R>   x;
    std::vector<int> cache;
    F<rec_t>         f;

    template <typename... Args>
    Catamorphism(IR::Tree const& _t, Args... args)
        : tree(_t), x(tree.size()), cache(tree.size()),
          f([&](int i) { return x[i]; }, args...)
    {
    }

    R operator()(int ref)
    {
        if (cache[ref]) return x[ref];
        cache[ref] = 1;
        x[ref]     = calculate(ref);
        return x[ref];
    }

  private:
    void ensure_children(int ref)
    {
        switch (static_cast<IRTag>(tree.get_type(ref))) {
        case IRTag::CONST:
        case IRTag::REG:
        case IRTag::TEMP:
        case IRTag::LABEL:
        case IRTag::JMP:
            break;

        case IRTag::BINOP: {
            auto [op, lhs, rhs] = tree.get_binop(ref);
            (*this)(lhs);
            (*this)(rhs);
        } break;
        case IRTag::MEM: {
            (*this)(tree.get_mem(ref).exp);
        } break;
        case IRTag::CALL: {
            for (int i : tree._explist[tree.get_call(ref).explist])
                (*this)(i);
        } break;
        case IRTag::MOVE: {
            auto [dst, src] = tree.get_move(ref);
            (*this)(dst);
            (*this)(src);
        } break;
        case IRTag::EXP: {
            (*this)(tree.get_exp(ref).exp);
        } break;
        case IRTag::CMP: {
            auto [lhs, rhs] = tree.get_cmp(ref);
            (*this)(lhs);
            (*this)(rhs);
        } break;
        case IRTag::CJMP: {
            auto [temp, target] = tree.get_cjmp(ref);
            (*this)(temp);
            (*this)(target);
        } break;
        case IRTag::PUSH: {
            (*this)(tree.get_push(ref).ref);
        } break;
        case IRTag::POP: {
            (*this)(tree.get_pop(ref).ref);
        } break;
        }
    }

    R calculate(int ref)
    {
        ensure_children(ref);
        switch (static_cast<IRTag>(tree.get_type(ref))) {
        case IRTag::CONST:
            return f(tree.get_const(ref));
        case IRTag::REG:
            return f(tree.get_reg(ref));
        case IRTag::TEMP:
            return f(tree.get_temp(ref));
        case IRTag::BINOP:
            return f(tree.get_binop(ref));
        case IRTag::MEM:
            return f(tree.get_mem(ref));
        case IRTag::CALL:
            return f(tree.get_call(ref));
        case IRTag::MOVE:
            return f(tree.get_move(ref));
        case IRTag::EXP:
            return f(tree.get_exp(ref));
        case IRTag::JMP:
            return f(tree.get_jmp(ref));
        case IRTag::LABEL:
            return f(tree.get_label(ref));
        case IRTag::CMP:
            return f(tree.get_cmp(ref));
        case IRTag::CJMP:
            return f(tree.get_cjmp(ref));
        case IRTag::PUSH:
            return f(tree.get_push(ref));
        case IRTag::POP:
            return f(tree.get_pop(ref));
        }
        __builtin_unreachable();
    }
};

template <typename FT>
void tree_dump(std::ostream& out, Tree const& tree, FT& F)
{
    Util::write(out, "Tree has", tree.size(), "nodes");
    for (int i = 0; i < static_cast<int>(tree.size()); i++)
        Util::write(out, "\t", i, ":\t", F(i));

    Util::write(out, "\nIt has", tree.methods.size(),
                "function fragments");
    for (auto const& f : tree.methods) {
        Util::write(out, f.first);
        if (!tree.aliases.count(f.first) ||
            tree.aliases.at(f.first).empty())
            Util::write(out, f.first, "has no aliases");
        else
            for (auto const& a : tree.aliases.at(f.first))
                Util::write(out, a, "is an alias of it");

        Util::write(out, "The arguments are:");
        int sp = f.second.stack.sp, tp = f.second.stack.tp;
        Util::write(out, "\t", "sp", ":=", "[", sp, "]\t", F(sp));
        Util::write(out, "\t", "tp", ":=", "[", tp, "]\t", F(tp));
        Util::write(out, "The spill size is",
                    f.second.stack.spill_size);
        for (auto const& a : f.second.stack.arguments)
            Util::write(out, "\t", a.first, ":=", "[", a.second, "]",
                        "\t", F(a.second));
        Util::write(out, "The code is:");
        for (auto const& s : f.second.stms)
            Util::write(out, "\t", s, ":\t", F(s));
    }

    Util::write(out, "\nIt has", tree._explist.size(), "Explists");
    for (int i = 0; i < static_cast<int>(tree._explist.size()); i++) {
        Util::write(out, "\t", std::to_string(i), ":\t");
        for (int j : tree.get_explist(i))
            Util::write(out, "\t\t", j, F(j));
        out << "\n";
    }
}

template <typename C> struct ShallowFormat {

    std::string operator()(Const const& c)
    {
        return std::string("CONST ") + std::to_string(c.value);
    }
    std::string operator()(Reg const& r)
    {
        return std::string("REG ") + std::to_string(r.id);
    }
    std::string operator()(Temp const& t)
    {
        return std::string("TEMP ") + std::to_string(t.id);
    }
    std::string operator()(Binop const& b)
    {
        static std::vector<std::string> names = {
            "+", "-", "*", "/", "&", "|", "<<", ">>", "ARSHIFT", "^"};
        return std::string("BINOP ") + names[b.op] +
               std::string(": ") + std::to_string(b.lhs) +
               std::string(" ") + std::to_string(b.rhs);
    }
    std::string operator()(Mem const& m)
    {
        return std::string("MEM ") + std::to_string(m.exp);
    }
    std::string operator()(Call const& c)
    {
        return std::string("CALL") + c.fn + std::string(" ") +
               std::to_string(c.explist);
    }
    std::string operator()(Cmp const& c)
    {
        return std::string("CMP ") + std::to_string(c.lhs) +
               std::string(" ") + std::to_string(c.rhs);
    }
    std::string operator()(Move const& m)
    {
        return std::string("MOVE ") + std::to_string(m.dst) +
               std::string(" ") + std::to_string(m.src);
    }
    std::string operator()(Exp const& e)
    {
        return std::string("EXP ") + std::to_string(e.exp);
    }
    std::string operator()(Jmp const& j)
    {
        return std::string("JMP ") + std::to_string(j.target);
    }
    std::string operator()(Cjmp const& c)
    {
        return std::string("CJMP ") + std::to_string(c.temp) +
               std::string(" ") + std::to_string(c.target);
    }
    std::string operator()(Label const& l)
    {
        return std::string("LABEL ") + std::to_string(l.id);
    }
    std::string operator()(Push const& p)
    {
        return std::string("PUSH ") + std::to_string(p.ref);
    }
    std::string operator()(Pop const& p)
    {
        return std::string("POP ") + std::to_string(p.ref);
    }

    ShallowFormat(C&& __fmap) : fmap(__fmap) {}
    C fmap;
};

template <typename C> struct DeepFormat {

    std::string operator()(Const const& c)
    {
        return std::string("CONST{") + std::to_string(c.value) +
               std::string("}");
    }
    std::string operator()(Reg const& r)
    {
        return std::string("REG{") + std::to_string(r.id) +
               std::string("}");
    }
    std::string operator()(Temp const& t)
    {
        return std::string("TEMP{") + std::to_string(t.id) +
               std::string("}");
    }
    std::string operator()(Binop const& b)
    {
        static std::vector<std::string> names = {
            "+", "-", "*", "/", "&", "|", "<<", ">>", "ARSHIFT", "^"};
        return std::string("BINOP{") + names[b.op] +
               std::string(", ") + fmap(b.lhs) + std::string(", ") +
               fmap(b.rhs) + std::string("}");
    }
    std::string operator()(Mem const& m)
    {
        return std::string("MEM{") + fmap(m.exp) + std::string("}");
    }
    std::string operator()(Call const& c)
    {
        return std::string("CALL{") + c.fn + std::string(", ") +
               std::to_string(c.explist) + std::string("}");
    }
    std::string operator()(Cmp const& c)
    {
        return std::string("CMP{") + fmap(c.lhs) + std::string(", ") +
               fmap(c.rhs) + std::string("}");
    }
    std::string operator()(Move const& m)
    {
        return std::string("MOVE{") + fmap(m.dst) +
               std::string(", ") + fmap(m.src) + std::string("}");
    }
    std::string operator()(Exp const& e)
    {
        return std::string("EXP{") + fmap(e.exp) + std::string("}");
    }
    std::string operator()(Jmp const& j)
    {
        return std::string("JMP{") + std::to_string(j.target) +
               std::string("}");
    }
    std::string operator()(Cjmp const& c)
    {
        return std::string("CJMP{") + fmap(c.temp) +
               std::string(", ") + fmap(c.target) + std::string("}");
    }
    std::string operator()(Label const& l)
    {
        return std::string("LABEL{") + std::to_string(l.id) +
               std::string("}");
    }
    std::string operator()(Push const& p)
    {
        return std::string("PUSH{") + fmap(p.ref) + std::string("}");
    }
    std::string operator()(Pop const& p)
    {
        return std::string("POP{") + fmap(p.ref) + std::string("}");
    }

    DeepFormat(C&& __fmap) : fmap(__fmap) {}
    C fmap;
};
} // namespace IR

#endif
