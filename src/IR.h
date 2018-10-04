#ifndef BCC_IR
#define BCC_IR

#include <cassert>
#include <functional>
#include <iostream>
#include <vector>

class IRBuilder;
namespace IR {

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

enum class ExpId { CONST, NAME, TEMP, BINOP, MEM, CALL, ESEQ };
enum class StmId { MOVE, EXP, JUMP, CJUMP, SEQ, LABEL };

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

struct Eseq {
    int stm;
    int exp;
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

struct Seq {
    int lhs;
    int rhs;
};

struct Label {
    int label;
};

class Tree {
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
    std::vector<Eseq>  _eseq;

    // Stm types
    std::vector<Move>  _move;
    std::vector<Exp>   _exp;
    std::vector<Jump>  _jump;
    std::vector<Cjump> _cjump;
    std::vector<Seq>   _seq;
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
    Eseq&  get_eseq(int ref);

    // Stm types
    Move&  get_move(int ref);
    Exp&   get_exp(int ref);
    Jump&  get_jump(int ref);
    Cjump& get_cjump(int ref);
    Seq&   get_seq(int ref);
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
        : tree(_t), x(tree.size()), f([&](int i) { return x[i]; }) {
        for (size_t i = 0; i < tree.size(); i++) x[i] = calculate(i);
    }

    R operator()(int ref) { return x[ref]; }

  private:
    R calculate(int ref) {
#define IR_DISPATCH(ID, name)                                        \
    case (ID):                                                       \
        return f(tree.get##name(ref));
        if (tree.get_type(ref) / 8 == 0) {
            switch (static_cast<ExpId>(tree.get_type(ref))) {
                IR_DISPATCH(ExpId::CONST, _const);
                IR_DISPATCH(ExpId::NAME, _name);
                IR_DISPATCH(ExpId::TEMP, _temp);
                IR_DISPATCH(ExpId::BINOP, _binop);
                IR_DISPATCH(ExpId::MEM, _mem);
                IR_DISPATCH(ExpId::CALL, _call);
                IR_DISPATCH(ExpId::ESEQ, _eseq);
            }
        }
        switch (static_cast<StmId>(tree.get_type(ref))) {
            IR_DISPATCH(StmId::MOVE, _move);
            IR_DISPATCH(StmId::EXP, _exp);
            IR_DISPATCH(StmId::JUMP, _jump);
            IR_DISPATCH(StmId::CJUMP, _cjump);
            IR_DISPATCH(StmId::SEQ, _seq);
            IR_DISPATCH(StmId::LABEL, _label);
        }
#undef IR_DISPATCH
        __builtin_unreachable();
    }
};
} // namespace IR

#endif
