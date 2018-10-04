#include "IRBuilder.h"

IRBuilder::IRBuilder(IR::Tree& tree) : base(tree), ds(0) {}

IRBuilder& IRBuilder::operator<<(IR::ExpId in) {
    kind = in;
    return *this;
}

IRBuilder& IRBuilder::operator<<(int in) {
    data[ds++] = in;
    return *this;
}

int IRBuilder::build() {
    base.kind.push_back(kind);
    ref = base.id++;
    switch (kind) {
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
            base._binop.push_back(IR::Binop{data[0],
                                            data[1], data[2]});
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
    return ref;
}
