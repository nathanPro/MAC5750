#include "IR.h"

static void check_for_exp(int found, int expected) {
    if (found / 8 != 0 || found % 8 != expected)
        throw IR::BadAccess{found, expected};
}

namespace IR {
    Const& Tree::get_const(int ref) {
        check_for_exp(kind[ref], IR::ExpId::CONST);
        return _const[pos[ref]];
    }

    Name& Tree::get_name(int ref) {
        check_for_exp(kind[ref], IR::ExpId::NAME);
        return _name[pos[ref]];
    }

    size_t Tree::size() const {
        return id;
    }

    int Tree::get_type(int ref) {
        return kind[ref];
    }
}
