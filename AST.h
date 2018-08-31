#include "string"
#include <variant>
#define UNREACHABLE() (__builtin_unreachable())

namespace AST {
enum class BinOp { And, Less, Plus, Minus, Times };

template <BinOp op> struct BinExp;
struct IndexingExp;
struct LengthExp;
struct MethodCallExp;
struct IntegerLiteralExp;
struct TrueExp;
struct FalseExp;
struct IdentifierExp;
struct ThisExp;
struct VectorExp;
struct ConstructorExp;
struct BangExp;
struct ParenExp;

// clang-format off
using Exp = std::variant<
    IntegerLiteralExp,
    TrueExp,
    FalseExp,
    ThisExp,
    IdentifierExp,
    BinExp<BinOp::And>, BinExp<BinOp::Less>, BinExp<BinOp::Plus>,
    BinExp<BinOp::Minus>, BinExp<BinOp::Times>,
    IndexingExp,
    LengthExp,
    MethodCallExp,
    VectorExp,
    ConstructorExp,
    BangExp,
    ParenExp
>;
// clang-format on

struct TrueExp {};
struct FalseExp {};
struct ThisExp {};

struct IdentifierExp {
    std::string value;
};

struct IntegerLiteralExp {
    double value = 0; // hue
};

struct ConstructorExp {
    std::string identifier;
};

template <BinOp op> struct BinExp {
    Exp* lhs = nullptr;
    Exp* rhs = nullptr;
};

struct IndexingExp {
    Exp* memory = nullptr;
    Exp* index  = nullptr;
};

struct LengthExp {
    Exp* exp = nullptr;
};

struct MethodCallExp {
    Exp* exp = nullptr;
    std::string identifier;
    // vector<ExpList*> arguments;
};

struct VectorExp {
    Exp* size = nullptr;
};

struct BangExp {
    Exp* exp = nullptr;
};

struct ParenExp {
    Exp* exp = nullptr;
};
} // namespace AST
