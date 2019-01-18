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

    std::vector<int>                             stm_seq;
    std::map<std::string, fragment>              methods;
    std::map<std::string, std::set<std::string>> aliases;
};

std::ostream& operator<<(std::ostream& out, Tree const&);

template <template <typename C> typename F, typename R>
struct Catamorphism {
    using rec_t = std::function<R(int)>;
    Tree const&    tree;
    std::vector<R> x;
    F<rec_t>       f;

    template <typename... Args>
    Catamorphism(IR::Tree const& _t, Args... args)
        : tree(_t), x(tree.size()),
          f([&](int i) { return x[i]; }, args...)
    {
        for (size_t i = 0; i < tree.size(); i++) x[i] = calculate(i);
    }

    R operator()(int ref) { return x[ref]; }

  private:
    R calculate(int ref)
    {
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
} // namespace IR

#endif
