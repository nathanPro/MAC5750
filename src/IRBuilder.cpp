#include "IRBuilder.h"

IRBuilder::IRBuilder(IR::Tree& tree) : base(tree), ds(0) {}

IRBuilder& IRBuilder::operator<<(IR::ExpId in) {
    kind = static_cast<int>(in);
    return *this;
}

IRBuilder& IRBuilder::operator<<(IR::StmId in) {
    kind = 8 + static_cast<int>(in);
    return *this;
}

IRBuilder& IRBuilder::operator<<(int in) {
    data[ds++] = in;
    return *this;
}

int IRBuilder::build() {
    base.kind.push_back(kind);
    ref = base.id++;
    if (kind / 8 == 0) {
        switch (static_cast<IR::ExpId>(kind)) {
        case IR::ExpId::CONST:
            base.pos.push_back(base._const.size());
            base._const.push_back(IR::Const{data[0]});
            break;
        case IR::ExpId::NAME:
            base.pos.push_back(base._name.size());
            base._name.push_back(IR::Name{data[0]});
            break;
        case IR::ExpId::TEMP:
            base.pos.push_back(base._temp.size());
            base._temp.push_back(IR::Temp{data[0]});
            break;
        case IR::ExpId::BINOP:
            base.pos.push_back(base._binop.size());
            base._binop.push_back(
                IR::Binop{data[0], data[1], data[2]});
            break;
        case IR::ExpId::MEM:
            base.pos.push_back(base._mem.size());
            base._mem.push_back(IR::Mem{data[0]});
            break;
        case IR::ExpId::CALL:
            base.pos.push_back(base._call.size());
            base._call.push_back(IR::Call{data[0], data[1]});
            break;
        case IR::ExpId::ESEQ:
            base.pos.push_back(base._eseq.size());
            base._eseq.push_back(IR::Eseq{data[0], data[1]});
            break;
        }
    } else {
        switch (static_cast<IR::StmId>(kind % 8)) {
        case IR::StmId::MOVE:
            base.pos.push_back(base._move.size());
            base._move.push_back(IR::Move{data[0], data[1]});
            break;
        case IR::StmId::EXP:
            base.pos.push_back(base._exp.size());
            base._exp.push_back(IR::Exp{data[0]});
            break;
        case IR::StmId::JUMP:
            base.pos.push_back(base._jump.size());
            base._jump.push_back(IR::Jump{data[0], data[1]});
            break;
        case IR::StmId::CJUMP:
            base.pos.push_back(base._cjump.size());
            base._cjump.push_back(IR::Cjump{data[0], data[1], data[2],
                                            data[3], data[4]});
            break;
        case IR::StmId::SEQ:
            base.pos.push_back(base._seq.size());
            base._seq.push_back(IR::Seq{data[0], data[1]});
            break;
        case IR::StmId::LABEL:
            base.pos.push_back(base._label.size());
            base._label.push_back(IR::Label{data[0]});
            break;
        }
    }
    return ref;
}
