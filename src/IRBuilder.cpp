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
            base.pos.emplace_back(base._const.size());
            base._const.push_back(IR::Const{data[0]});
        break;
        case IR::ExpId::NAME:
            base.pos.emplace_back(base._name.size());
            base._name.push_back(IR::Name{data[0]});
        break;
    }
    return ref;
}
