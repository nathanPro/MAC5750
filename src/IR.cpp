#include "IR.h"

namespace IR {
    Const& Tree::get_const(int ref) {
        assert(kind[ref] / 8 == 0);
        assert(kind[ref] == IR::ExpId::CONST);
        return _const[pos[ref]];
    }

    Name& Tree::get_name(int ref) {
        assert(kind[ref] / 8 == 0);
        assert(kind[ref] == IR::ExpId::NAME);
        return _name[pos[ref]];
    }

    size_t Tree::size() const {
        return id;
    }

    int Tree::get_type(int ref) {
        return kind[ref];
    }
}
