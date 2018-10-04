#ifndef BCC_IR
#define BCC_IR

#include <vector>
#include <cassert>

class IRBuilder;
namespace IR {

struct BadAccess {
    int found;
    int expected;
};

enum ExpId
{
    CONST,
    NAME,
    TEMP,
    BINOP,
    MEM,
    CALL,
    ESEQ
};

enum BinOp {
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

class Tree {
    friend class ::IRBuilder;
    size_t id;
    std::vector<int> kind;
    std::vector<int> pos;
    std::vector<Const> _const;
    std::vector<Name>  _name;
    std::vector<Temp>  _temp;
    std::vector<Binop> _binop;
    std::vector<Mem>   _mem;
    std::vector<Call>  _call;
    std::vector<Eseq>  _eseq;

  public:
    Tree() : id(0) {}
    Const& get_const(int ref);
    Name&  get_name(int ref);
    Temp&  get_temp(int ref);
    Binop& get_binop(int ref);
    Mem&   get_mem(int ref);
    Call&  get_call(int ref);
    Eseq&  get_eseq(int ref);
    int get_type(int ref);
    size_t size() const;
};
}

#endif
