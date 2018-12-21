#include "IR.h"

static void check_id(int found, int expected)
{
    if (found != expected) throw IR::BadAccess{found, expected};
}

namespace IR
{
#define IR_GETTER(name, ID, ret)                                     \
    ret& Tree::get##name(int ref)                                    \
    {                                                                \
        check_id(kind[ref], static_cast<int>(ID));                   \
        return name[pos[ref]];                                       \
    }

IR_GETTER(_const, IRTag::CONST, Const)
IR_GETTER(_name, IRTag::NAME, Name)
IR_GETTER(_temp, IRTag::TEMP, Temp)
IR_GETTER(_binop, IRTag::BINOP, Binop)
IR_GETTER(_mem, IRTag::MEM, Mem)
IR_GETTER(_call, IRTag::CALL, Call)
IR_GETTER(_move, IRTag::MOVE, Move)
IR_GETTER(_exp, IRTag::EXP, Exp)
IR_GETTER(_jump, IRTag::JUMP, Jump)
IR_GETTER(_label, IRTag::LABEL, Label)

#undef IR_GETTER
size_t Tree::size() const { return id; }

IRTag Tree::get_type(int ref)
{
    return static_cast<IRTag>(kind[ref]);
}

Explist Tree::get_explist(int ref) { return _explist[ref]; }

int Tree::keep_explist(Explist&& els)
{
    int ans = _explist.size();
    _explist.push_back(els);
    return ans;
}
} // namespace IR
