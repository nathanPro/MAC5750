#ifndef BCC_GRAMMAR
#define BCC_GRAMMAR

#include <memory>
#include <utility>
#include <variant>

template <typename T> class Entity {
    uint32_t tag;

  public:
    explicit Entity(uint32_t val) : tag(val) {}
    uint32_t get() const { return tag; }
    Entity   operator++() { return Entity(1 + tag); }
};

namespace Grammar {
template <typename variant> struct Nonterminal {
    using variant_t = variant;

    template <typename T>
    Nonterminal(T&& in)
        : _self(std::make_unique<variant_t>(std::move(in))) {}
    template <class Visitor, class Nonterminal>
    friend constexpr auto visit(Visitor&& vis, Nonterminal&& nt);
    template <typename Alternative, class Nonterminal>
    friend constexpr const Alternative& get(Nonterminal&&);

  private:
    std::unique_ptr<variant_t> _self;
};

template <class Visitor, class Nonterminal>
constexpr auto visit(Visitor&& vis, Nonterminal&& nt) {
    return std::visit(std::forward<Visitor>(vis), *nt._self);
}

template <typename Alternative, class Nonterminal>
constexpr const Alternative& get(Nonterminal&& nt) {
    return std::get<Alternative>(*nt._self);
}
}; // namespace Grammar

#endif
