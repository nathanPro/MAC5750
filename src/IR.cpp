#include "IR.h"

static void check_id(int found, int expected)
{
    if (found != expected) throw IR::BadAccess{found, expected};
}

namespace IR
{
#define IR_GETTER(name, ID, ret)                                     \
    ret& Tree::get##name(int ref)                                    \
    {                                                                \
        check_id(kind[ref], static_cast<int>(ID));                   \
        return name[pos[ref]];                                       \
    }

IR_GETTER(_const, IRTag::CONST, Const)
IR_GETTER(_name, IRTag::NAME, Name)
IR_GETTER(_temp, IRTag::TEMP, Temp)
IR_GETTER(_binop, IRTag::BINOP, Binop)
IR_GETTER(_mem, IRTag::MEM, Mem)
IR_GETTER(_call, IRTag::CALL, Call)
IR_GETTER(_move, IRTag::MOVE, Move)
IR_GETTER(_exp, IRTag::EXP, Exp)
IR_GETTER(_jmp, IRTag::JMP, Jmp)
IR_GETTER(_label, IRTag::LABEL, Label)
IR_GETTER(_cmp, IRTag::CMP, Cmp)
IR_GETTER(_cjmp, IRTag::CJMP, Cjmp)

#undef IR_GETTER
size_t Tree::size() const { return id; }

IRTag Tree::get_type(int ref)
{
    return static_cast<IRTag>(kind[ref]);
}

Explist Tree::get_explist(int ref) { return _explist[ref]; }

int Tree::keep_explist(Explist&& els)
{
    int ans = _explist.size();
    _explist.push_back(els);
    return ans;
}

template <typename C> struct Inners {

    std::string operator()(Const const& c)
    {
        return std::to_string(c.value);
    }
    std::string operator()(Name const& n)
    {
        return std::to_string(n.id);
    }
    std::string operator()(Temp const& t)
    {
        return std::to_string(t.id);
    }
    std::string operator()(Binop const& b)
    {
        static std::vector<std::string> names = {
            "+", "-", "*", "/", "&", "|", "<<", ">>", "ARSHIFT", "^"};
        return names[b.op] + std::string(": ") +
               std::to_string(b.lhs) + std::string(", ") +
               std::to_string(b.rhs);
    }
    std::string operator()(Mem const& m)
    {
        return std::to_string(m.exp);
    }
    std::string operator()(Call const& c)
    {
        return std::to_string(c.fn) + std::string(", ") +
               std::to_string(c.explist);
    }
    std::string operator()(Cmp const& c)
    {
        return std::to_string(c.lhs) + std::string(", ") +
               std::to_string(c.rhs);
    }
    std::string operator()(Move const& m)
    {
        return std::to_string(m.dst) + std::string(", ") +
               std::to_string(m.src);
    }
    std::string operator()(Exp const& e)
    {
        return std::to_string(e.exp);
    }
    std::string operator()(Jmp const& j)
    {
        return std::to_string(j.target);
    }
    std::string operator()(Cjmp const& c)
    {
        static std::vector<std::string> names = {
            "==", "!=", "<",   ">",  "<=",
            ">=", "u<", "u<=", "u>", "u>="};
        return names[c.op] + std::string(": ") +
               std::to_string(c.temp) + std::string(", ") +
               std::to_string(c.target);
    }
    std::string operator()(Label const& l)
    {
        return std::to_string(l.id);
    }

    Inners(C&& __fmap) : fmap(__fmap) {}
    C fmap;
};

std::ostream& operator<<(std::ostream& out, Tree& t)
{
    std::vector<std::string> TYPES = {
        "CONST", "NAME", "TEMP", "BINOP", "MEM",   "CALL",
        "CMP",   "MOVE", "EXP",  "JMP",   "LABEL", "CJMP"};

    IR::Catamorphism<Inners, std::string> F(t);

    Util::write(out, "Tree has", t.pos.size(), "nodes");
    for (int i = 0; i < t.kind.size(); i++)
        Util::write(out, "[", i, "]",
                    TYPES[static_cast<int>(t.get_type(i))], F(i));

    Util::write(out, "It has", t.methods.size(),
                "function fragments");

    for (auto const& f : t.methods) Util::write(out, f.first);
    return out;
}
} // namespace IR
