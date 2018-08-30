#include "util.h"
#include "AST.h"

struct Eval {
    int32_t operator()(const IntExp& exp) { return exp.val; }

    int32_t operator()(const ProdExp& exp) {
        return std::visit(*this, *exp.lhs)
            *  std::visit(*this, *exp.rhs);
    }

    int32_t operator()(const SumExp& exp) {
        return std::visit(*this, *exp.lhs)
            +  std::visit(*this, *exp.rhs);
    }
};

int main() {
    Exp x(IntExp{10});
    Exp r(SumExp{&x, &x});
    write(std::cout, std::visit(Eval(), x));
    write(std::cout, std::visit(Eval(), r));
}
