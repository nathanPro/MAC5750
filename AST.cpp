#include "AST.h"

int main() {
    auto lhs  = Tiger::NumExp(38);
    auto rhs  = Tiger::NumExp(12);
    auto sum  = Tiger::BinExp(&lhs, &rhs);
    auto prd  = Tiger::BinExp<Tiger::BinOp::Times>(&lhs, &rhs);
    auto tail = Tiger::LastExpList(&prd);
    auto head = Tiger::PairExpList(&sum, &tail);
    auto root = Tiger::PrintStm(&head);
    Tiger::Stm::Env env;

    root.eval(env);
}
