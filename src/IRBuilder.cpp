#include "IRBuilder.h"

IRBuilder::IRBuilder(IR::Tree& tree) : base(tree), ds(0) {}

IRBuilder& IRBuilder::operator<<(IR::IRTag in)
{
    kind = static_cast<int>(in);
    return *this;
}

IRBuilder& IRBuilder::operator<<(int in)
{
    data[ds++] = in;
    return *this;
}

int IRBuilder::build()
{
    base.kind.push_back(kind);
    ref = base.id++;
    switch (static_cast<IR::IRTag>(kind)) {
    case IR::IRTag::CONST:
        base.pos.push_back(base._const.size());
        base._const.push_back(IR::Const{data[0]});
        break;
    case IR::IRTag::NAME:
        base.pos.push_back(base._name.size());
        base._name.push_back(IR::Name{data[0]});
        break;
    case IR::IRTag::TEMP:
        base.pos.push_back(base._temp.size());
        base._temp.push_back(IR::Temp{data[0]});
        break;
    case IR::IRTag::BINOP:
        base.pos.push_back(base._binop.size());
        base._binop.push_back(IR::Binop{data[0], data[1], data[2]});
        break;
    case IR::IRTag::MEM:
        base.pos.push_back(base._mem.size());
        base._mem.push_back(IR::Mem{data[0]});
        break;
    case IR::IRTag::CALL:
        base.pos.push_back(base._call.size());
        base._call.push_back(IR::Call{data[0], data[1]});
        break;
    case IR::IRTag::MOVE:
        base.pos.push_back(base._move.size());
        base._move.push_back(IR::Move{data[0], data[1]});
        break;
    case IR::IRTag::EXP:
        base.pos.push_back(base._exp.size());
        base._exp.push_back(IR::Exp{data[0]});
        break;
    case IR::IRTag::JUMP:
        base.pos.push_back(base._jump.size());
        base._jump.push_back(IR::Jump{data[0], data[1]});
        break;
    case IR::IRTag::CJUMP:
        base.pos.push_back(base._cjump.size());
        base._cjump.push_back(
            IR::Cjump{data[0], data[1], data[2], data[3], data[4]});
        break;
    case IR::IRTag::SEQ:
        base.pos.push_back(base._seq.size());
        base._seq.push_back(IR::Seq{data[0], data[1]});
        break;
    case IR::IRTag::LABEL:
        base.pos.push_back(base._label.size());
        base._label.push_back(IR::Label{data[0]});
        break;
    }
    return ref;
}
