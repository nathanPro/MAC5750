#include "IR.h"

static void check_for_exp(int found, int expected) {
    if (found / 8 != 0 || found % 8 != expected)
        throw IR::BadAccess{found, expected};
}

namespace IR {
#define IR_GETTER(name, ID, ret) \
    ret& Tree::get ## name (int ref) { \
        check_for_exp(kind[ref], ID); \
        return name[pos[ref]]; \
    }

    IR_GETTER(_const, ExpId::CONST, Const)
    IR_GETTER(_name,  ExpId::NAME,  Name)
    IR_GETTER(_temp,  ExpId::TEMP,  Temp)
    IR_GETTER(_binop, ExpId::BINOP, Binop)
    IR_GETTER(_mem,   ExpId::MEM,     Mem)
    IR_GETTER(_call,  ExpId::CALL,   Call)
    IR_GETTER(_eseq,  ExpId::ESEQ,   Eseq)

#undef IR_GETTER

    size_t Tree::size() const {
        return id;
    }

    int Tree::get_type(int ref) {
        return kind[ref];
    }
}
