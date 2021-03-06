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

IRBuilder& IRBuilder::operator<<(std::string _s)
{
    s = _s;
    return *this;
}

int store_in_temp(IR::Tree& t, int exp_ref)
{
    auto tref = t.new_temp();

    IRBuilder assign(t);
    assign << IR::IRTag::MOVE << tref << exp_ref;
    assign.build();

    return tref;
}

int IRBuilder::build()
{
    if (static_cast<IR::IRTag>(kind) == IR::IRTag::LABEL) return -1;
    base.kind.push_back(kind);
    ref = base.pos.size();
    switch (static_cast<IR::IRTag>(kind)) {
    case IR::IRTag::CONST:
        base.pos.push_back(base._const.size());
        base._const.push_back(IR::Const{data[0]});
        break;
    case IR::IRTag::REG:
        base.pos.push_back(base._reg.size());
        base._reg.push_back(IR::Reg{data[0]});
        break;
    case IR::IRTag::TEMP:
        base.pos.push_back(base._temp.size());
        base._temp.push_back(IR::Temp{base.tmp++});
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
        base._call.push_back(IR::Call{s, data[0]});
        break;
    case IR::IRTag::CMP:
        base.pos.push_back(base._cmp.size());
        base._cmp.push_back(IR::Cmp{data[0], data[1]});
        break;
    case IR::IRTag::MOVE:
        base.stm_seq.push_back(ref);
        base.pos.push_back(base._move.size());
        data[0] = [&](IR::IRTag tag) {
            if (tag == IR::IRTag::TEMP || tag == IR::IRTag::MEM ||
                tag == IR::IRTag::REG)
                return data[0];
            else {
                IRBuilder mem(base);
                mem << IR::IRTag::MEM << data[0];
                return mem.build();
            }
        }(base.get_type(data[0]));
        base._move.push_back(IR::Move{data[0], data[1]});
        break;
    case IR::IRTag::EXP:
        base.stm_seq.push_back(ref);
        base.pos.push_back(base._exp.size());
        base._exp.push_back(IR::Exp{data[0]});
        break;
    case IR::IRTag::JMP:
        base.stm_seq.push_back(ref);
        base.pos.push_back(base._jmp.size());
        base._jmp.push_back(IR::Jmp{data[0]});
        break;
    case IR::IRTag::CJMP:
        base.stm_seq.push_back(ref);
        base.pos.push_back(base._cjmp.size());
        base._cjmp.push_back(IR::Cjmp{data[0], data[1]});
        break;
    case IR::IRTag::PUSH:
        base.stm_seq.push_back(ref);
        base.pos.push_back(base._push.size());
        base._push.push_back(IR::Push{data[0]});
        break;
    case IR::IRTag::POP:
        base.stm_seq.push_back(ref);
        base.pos.push_back(base._pop.size());
        base._pop.push_back(IR::Pop{data[0]});
        break;
    case IR::IRTag::LABEL:
        break;
    }
    return ref;
}
