#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace AST {
class Builder {
    std::tuple<std::vector<int>, std::vector<std::string>> inner;

  public:
    template <typename T> Builder& operator<<(T&& in) {
        using U = std::vector<std::decay_t<T>>;
        std::get<U>(inner).push_back(in);
        return *this;
    }

    template <typename T> friend std::vector<T> claim(Builder& data);
};

template <typename T> std::vector<T> claim(Builder& data) {
    using U = std::vector<std::decay_t<T>>;
    return std::move(std::get<U>(data.inner));
}

class Exp {
    int sum;

  public:
    Exp() {}
    Exp(Builder&& data) : sum(0) {
        auto tmp = claim<int>(data);
        sum      = std::accumulate(std::begin(tmp), std::end(tmp), 0);
    }

    operator int() { return sum; }
};

class Stm {
    std::string sum;

  public:
    Stm() {}
    Stm(Builder&& data) {
        auto tmp = claim<std::string>(data);
        sum      = tmp.at(0) + tmp.at(0);
    }

    operator std::string() { return sum; }
};

using Root = std::variant<Exp, Stm>;

template <typename T> std::unique_ptr<Root> make(Builder&& data) {
    return std::make_unique<Root>(T(std::move(data)));
}
} // namespace AST

int main() {
    int n;
    std::cin >> n;

    AST::Builder keep;
    for (int i = 0; i < n; i++) {
        int         x;
        std::string s;

        std::cin >> x >> s;
        keep << x << s;
    }

    auto handle_e = AST::make<AST::Exp>(std::move(keep));
    auto handle_s = AST::make<AST::Stm>(std::move(keep));

    std::cout << int(std::get<AST::Exp>(*handle_e)) << std::endl;
    std::cout << std::string(std::get<AST::Stm>(*handle_s))
              << std::endl;
    return 0;
}
