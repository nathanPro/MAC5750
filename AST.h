#include "experimental/optional"
#include "iostream"
#include "map"
#include "tuple"
#define UNREACHABLE() (__builtin_unreachable())

namespace Tiger {
template <typename T> using Opt = std::experimental::optional<T>;
template <typename Result, typename Environment> struct Symbol {
    using Env = Environment;
    using Ret = std::pair<Opt<Result>, Environment>;
    virtual Ret eval(Environment) const = 0;
};

using Base = Symbol<int32_t, std::map<std::string, int32_t>>;

struct Exp : Base {};
struct Stm : Base {};
struct ExpList : Base {};

struct NumExp : Exp {
    int32_t val;
    NumExp(int32_t _v) : val(_v) {}
    Exp::Ret eval(Exp::Env) const override;
};

struct IdExp : Exp {
    std::string identifier;
    IdExp(std::string id) : identifier(id) {}
    Exp::Ret eval(Exp::Env) const override;
};

struct EseqExp : Exp {
    Stm* stm = nullptr;
    Exp* exp = nullptr;
    EseqExp(Stm* s, Exp* e) : stm(s), exp(e) {}
    Exp::Ret eval(Exp::Env) const override;
};

enum class BinOp { Plus, Minus, Times, Div };

template <BinOp op = BinOp::Plus> struct BinExp : Exp {
    Exp* lhs = nullptr;
    Exp* rhs = nullptr;
    BinExp(Exp* _lhs, Exp* _rhs) : lhs(_lhs), rhs(_rhs) {}
    BinExp::Ret eval(BinExp::Env env) const override;
};

struct PairExpList : ExpList {
    Exp* head     = nullptr;
    ExpList* tail = nullptr;
    PairExpList(Exp* h, ExpList* t) : head(h), tail(t) {}
    ExpList::Ret eval(ExpList::Env) const override;
};

struct LastExpList : ExpList {
    Exp* exp = nullptr;
    LastExpList(Exp* e) : exp(e) {}
    ExpList::Ret eval(ExpList::Env) const override;
};

struct CompoundStm : Stm {
    Stm* lhs = nullptr;
    Stm* rhs = nullptr;
    CompoundStm(Stm* l, Stm* r) : lhs(l), rhs(r) {}
    Stm::Ret eval(Stm::Env) const override;
};

struct AssignStm : Stm {
    std::string identifier;
    Exp* exp = nullptr;
    AssignStm(std::string id, Exp* e) : identifier(id), exp(e) {}
    Stm::Ret eval(Stm::Env) const override;
};

struct PrintStm : Stm {
    ExpList* list = nullptr;
    PrintStm(ExpList* l) : list(l) {}
    Stm::Ret eval(Stm::Env) const override;
};

// Evaluations of expressions
Exp::Ret NumExp::eval(Exp::Env env) const { return {val, env}; }

Exp::Ret IdExp::eval(Exp::Env env) const {
    return {env.at(identifier), env};
}

Exp::Ret EseqExp::eval(Exp::Env env) const {
    auto [_, senv] = stm->eval(env);
    return exp->eval(senv);
}

template <BinOp op>
typename BinExp<op>::Ret
BinExp<op>::eval(typename BinExp<op>::Env env) const {
    auto [lval, lenv] = lhs->eval(env);
    auto [rval, renv] = rhs->eval(env);
    auto els          = lval.value();
    auto ers          = rval.value();
    switch (op) {
    case BinOp::Plus:
        return {els + ers, env};
    case BinOp::Minus:
        return {els - ers, env};
    case BinOp::Times:
        return {els * ers, env};
    case BinOp::Div:
        return {els / ers, env};
    }
    UNREACHABLE();
}

ExpList::Ret PairExpList::eval(ExpList::Env env) const {
    head->eval(env);
    return tail->eval(env);
}

ExpList::Ret LastExpList::eval(ExpList::Env env) const {
    return exp->eval(env);
}

Stm::Ret CompoundStm::eval(Stm::Env env) const {
    auto lenv = lhs->eval(env).second;
    auto renv = rhs->eval(lenv).second;
    return {{}, renv};
}

Stm::Ret AssignStm::eval(Stm::Env env) const {
    auto [val, nenv] = exp->eval(env);
    nenv[identifier] = val.value();
    return {{}, nenv};
}

Stm::Ret PrintStm::eval(Stm::Env env) const {
    ExpList* it = list;
    while (auto curr = dynamic_cast<PairExpList*>(it)) {
        std::cout << curr->head->eval(env).first.value() << ' ';
        it = curr->tail;
    }
    if (!it) return {{}, env};
    auto curr = dynamic_cast<LastExpList*>(it);
    std::cout << curr->exp->eval(env).first.value() << '\n';
    return {{}, env};
}
} // namespace Tiger
