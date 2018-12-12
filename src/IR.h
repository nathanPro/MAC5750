#ifndef BCC_IR
#define BCC_IR

#include <cassert>
#include <functional>
#include <iostream>
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
    NAME,
    TEMP,
    BINOP,
    MEM,
    CALL,
    MOVE,
    EXP,
    JUMP,
    CJUMP,
    LABEL
};

struct Const {
    int value;
};

struct Name {
    int label;
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
    int func;
    int explist;
};

struct Move {
    int dst;
    int src;
};

struct Exp {
    int exp;
};

struct Jump {
    int exp;
    int targets;
};

struct Cjump {
    int op;
    int lhs;
    int rhs;
    int iftrue;
    int iffalse;
};

struct Label {
    int label;
};

class Tree
{
    friend class ::IRBuilder;
    // Type checking info
    size_t           id;
    std::vector<int> kind;
    std::vector<int> pos;

    // Exp types
    std::vector<Const> _const;
    std::vector<Name>  _name;
    std::vector<Temp>  _temp;
    std::vector<Binop> _binop;
    std::vector<Mem>   _mem;
    std::vector<Call>  _call;

    // Stm types
    std::vector<Move>  _move;
    std::vector<Exp>   _exp;
    std::vector<Jump>  _jump;
    std::vector<Cjump> _cjump;
    std::vector<Label> _label;

  public:
    Tree() : id(0) {}
    // Exp types
    Const& get_const(int ref);
    Name&  get_name(int ref);
    Temp&  get_temp(int ref);
    Binop& get_binop(int ref);
    Mem&   get_mem(int ref);
    Call&  get_call(int ref);

    // Stm types
    Move&  get_move(int ref);
    Exp&   get_exp(int ref);
    Jump&  get_jump(int ref);
    Cjump& get_cjump(int ref);
    Label& get_label(int ref);

    // generic functinality
    int    get_type(int ref);
    size_t size() const;
};

template <template <typename C> typename F, typename R>
struct Catamorphism {
    using rec_t = std::function<R(int)>;
    Tree&          tree;
    std::vector<R> x;
    F<rec_t>       f;

    Catamorphism(IR::Tree& _t)
        : tree(_t), x(tree.size()), f([&](int i) { return x[i]; })
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
        case IRTag::NAME:
            return f(tree.get_name(ref));
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
        case IRTag::JUMP:
            return f(tree.get_jump(ref));
        case IRTag::CJUMP:
            return f(tree.get_cjump(ref));
        case IRTag::LABEL:
            return f(tree.get_label(ref));
        }
        __builtin_unreachable();
    }
};
} // namespace IR

#endif
