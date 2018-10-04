#include "IR.h"

static void check_for_exp(int found, int expected) {
    if (found / 8 != 0 || found % 8 != expected)
        throw IR::BadAccess{found, expected};
}

static void check_for_stm(int found, int expected) {
    if (found / 8 != 1 || found % 8 != expected)
        throw IR::BadAccess{found, expected};
}

namespace IR {
#define IR_GETTER(name, ID, ret)                                     \
    ret& Tree::get##name(int ref) {                                  \
        check_for_exp(kind[ref], static_cast<int>(ID));              \
        return name[pos[ref]];                                       \
    }

IR_GETTER(_const, ExpId::CONST, Const)
IR_GETTER(_name, ExpId::NAME, Name)
IR_GETTER(_temp, ExpId::TEMP, Temp)
IR_GETTER(_binop, ExpId::BINOP, Binop)
IR_GETTER(_mem, ExpId::MEM, Mem)
IR_GETTER(_call, ExpId::CALL, Call)
IR_GETTER(_eseq, ExpId::ESEQ, Eseq)

#undef IR_GETTER
#define IR_GETTER(name, ID, ret)                                     \
    ret& Tree::get##name(int ref) {                                  \
        check_for_stm(kind[ref], static_cast<int>(ID));              \
        return name[pos[ref]];                                       \
    }
IR_GETTER(_move, StmId::MOVE, Move);
IR_GETTER(_exp, StmId::EXP, Exp);
IR_GETTER(_jump, StmId::JUMP, Jump);
IR_GETTER(_cjump, StmId::CJUMP, Cjump);
IR_GETTER(_seq, StmId::SEQ, Seq);
IR_GETTER(_label, StmId::LABEL, Label);

#undef IR_GETTER
size_t Tree::size() const { return id; }

int Tree::get_type(int ref) { return kind[ref]; }
} // namespace IR
