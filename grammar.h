#ifndef BCC_GRAMMAR
#define BCC_GRAMMAR

#include <memory>
#include <utility>
#include <variant>

namespace Grammar {
template <typename tag, typename PtrL, typename PtrR = PtrL>
struct BinaryRule {
    using E = typename std::pointer_traits<PtrL>::pointer;
    using F = typename std::pointer_traits<PtrR>::pointer;
    E lhs   = nullptr;
    F rhs   = nullptr;
};

template <typename tag, typename Ptr> struct UnaryRule {
    using E = typename std::pointer_traits<Ptr>::pointer;
    E inner = nullptr;
};

template <typename variant> struct Nonterminal {
    using variant_t = variant;

    Nonterminal(variant&& in) : __content(std::move(in)) {}
    template <class Visitor, class Nonterminal>
    friend constexpr auto visit(Visitor&& vis, Nonterminal&& nt);
    template <typename Alternative, class Nonterminal>
    friend constexpr const Alternative& get(Nonterminal&&);

  private:
    variant __content;

    // TODO Can I inherit from BinaryRule to enhance a type?
};

template <class Visitor, class Nonterminal>
constexpr auto visit(Visitor&& vis, Nonterminal&& nt) {
    return std::visit(std::forward<Visitor>(vis), nt.__content);
}


template <typename Alternative, class Nonterminal>
constexpr const Alternative& get(Nonterminal&& nt) {
    return std::get<Alternative>(nt.__content);
}
}; // namespace Grammar

#endif
