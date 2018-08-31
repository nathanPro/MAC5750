#include "AST.h"
#include "iostream"
#include "map"
#include "util.h"

class Eval {
    std::map<std::string, double> env;

  public:
    Eval(std::map<std::string, double>& _env) : env(_env) {}

    template <AST::BinOp op>
    double operator()(const AST::BinExp<op>& exp) {
        auto lhs = std::visit(*this, *exp.lhs);
        auto rhs = std::visit(*this, *exp.rhs);
        switch (op) {
        case AST::BinOp::Plus:
            return lhs + rhs;
        case AST::BinOp::Minus:
            return lhs - rhs;
        case AST::BinOp::Times:
            return lhs * rhs;
        default:
            return 0;
        }
    }

    double operator()(const AST::IntegerLiteralExp& exp) {
        return exp.value;
    }

    double operator()(const AST::IdentifierExp& exp) {
        return env.at(exp.value);
    }

    double operator()(const AST::TrueExp& exp) { return 1; }
    double operator()(const AST::FalseExp& exp) { return 0; }
    double operator()(const AST::BangExp& exp) {
        return !std::visit(*this, *exp.exp);
    }
    double operator()(const AST::ParenExp& exp) {
        return std::visit(*this, *exp.exp);
    }

    double operator()(const AST::ThisExp& exp) { return -1; }
    double operator()(const AST::IndexingExp& exp) { return -1; }
    double operator()(const AST::LengthExp& exp) { return -1; }
    double operator()(const AST::MethodCallExp& exp) { return -1; }
    double operator()(const AST::VectorExp& exp) { return -1; }
    double operator()(const AST::ConstructorExp& exp) { return -1; }
};

int main() {
    auto x = AST::Exp(AST::IdentifierExp{std::string{"x"}});
    auto double_x = AST::Exp(AST::BinExp<AST::BinOp::Plus>{&x, &x});
    auto four_x = AST::Exp(AST::BinExp<AST::BinOp::Plus>{&double_x, &double_x});

    double in;
    while (std::cin >> in) {
        std::map<std::string, double> env;
        env["x"] = in;
        write(std::cout, std::visit(Eval(env), four_x));
    }
}
