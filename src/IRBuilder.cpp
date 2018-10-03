#include "IRBuilder.h"

IRBuilder::IRBuilder(IR::Tree& tree) : base(tree), ref(base.id++) {
    base.kind.emplace_back();
    base.pos.emplace_back();
}

IRBuilder::operator int() {
    return ref;
}

IRBuilder& IRBuilder::operator<<(IR::ExpId in) {
    base.kind[ref] = in;
    switch (in) {
        case IR::ExpId::CONST:
            base.pos[ref] = base._const.size();
            base._const.emplace_back();
            break;
    }
    return *this;
}

IRBuilder& IRBuilder::operator<<(int in) {
    const auto& kind = base.kind[ref];
    if (kind / 8) {
    } else {
        switch (kind) {
            case IR::ExpId::CONST:
                base.get_const(ref).value = in;
        }
    }
    return *this;
}
