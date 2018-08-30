#include <variant>
#define UNREACHABLE() (__builtin_unreachable())

struct SumExp;
struct ProdExp;
struct IntExp;

using Exp = std::variant<SumExp, ProdExp, IntExp>;

struct SumExp {
    Exp* lhs = nullptr;
    Exp* rhs = nullptr;
};

struct ProdExp {
    Exp* lhs = nullptr;
    Exp* rhs = nullptr;
};

struct IntExp {
    int32_t val;
};
