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

Tree::Tree() : tmp(0), lbl(0) {}

size_t Tree::size() const { return pos.size(); }

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

int Tree::new_temp()
{
    auto ref = pos.size();
    kind.push_back(static_cast<int>(IRTag::TEMP));
    pos.push_back(_temp.size());
    _temp.push_back(Temp{tmp++});
    return ref;
}

label_handle::label_handle(int _ref) : ref(_ref) {}
label_handle::operator int() const { return ref; }

label_handle Tree::new_label()
{
    auto ref = pos.size();
    kind.push_back(static_cast<int>(IRTag::LABEL));
    pos.push_back(_label.size());
    _label.push_back(Label{lbl++});
    return ref;
}

int Tree::place_label(label_handle&& lbl)
{
    stm_seq.push_back(lbl.ref);
    return lbl.ref;
}

size_t fragment::size() const { return stms.size(); }

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
               std::to_string(b.lhs) + std::string(" ") +
               std::to_string(b.rhs);
    }
    std::string operator()(Mem const& m)
    {
        return std::to_string(m.exp);
    }
    std::string operator()(Call const& c)
    {
        return c.fn + std::string(" ") + std::to_string(c.explist);
    }
    std::string operator()(Cmp const& c)
    {
        return std::to_string(c.lhs) + std::string(" ") +
               std::to_string(c.rhs);
    }
    std::string operator()(Move const& m)
    {
        return std::to_string(m.dst) + std::string(" ") +
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
        return std::to_string(c.temp) + std::string(" ") +
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
    for (int i = 0; i < static_cast<int>(t.kind.size()); i++)
        Util::write(out, "\t", i, ":\t",
                    TYPES[static_cast<int>(t.get_type(i))], "\t",
                    F(i));

    Util::write(out, "\nIt has", t.methods.size(),
                "function fragments");
    for (auto const& f : t.methods) {
        Util::write(out, f.first);
        Util::write(out, "The arguments are:");
        if (t.aliases[f.first].empty())
            Util::write(out, f.first, "has no aliases");
        else
            for (auto const& a : t.aliases[f.first])
                Util::write(out, a, "is an alias of it");

        int sp = f.second.stack.sp, tp = f.second.stack.tp;
        Util::write(out, "\t", "sp", ":=", "[", sp, "]",
                    TYPES[static_cast<int>(t.get_type(sp))], "\t",
                    t.get_temp(sp).id);
        Util::write(out, "\t", "tp", ":=", "[", tp, "]",
                    TYPES[static_cast<int>(t.get_type(tp))], "\t",
                    t.get_temp(tp).id);
        for (auto const& a : f.second.stack.arguments)
            Util::write(out, "\t", a.first, ":=", "[", a.second, "]",
                        TYPES[static_cast<int>(t.get_type(a.second))],
                        "\t", t.get_temp(a.second).id);
        Util::write(out, "The code is:");
        for (auto const& s : f.second.stms)
            Util::write(out, "\t", s, ":\t",
                        TYPES[static_cast<int>(t.get_type(s))], "\t",
                        F(s));
    }

    Util::write(out, "\nIt has", t._explist.size(), "Explists");
    for (int i = 0; i < static_cast<int>(t._explist.size()); i++) {
        out << std::string("\t")
            << std::to_string(i) + std::string(":\t");
        for (int j : t.get_explist(i)) out << j << " ";
        out << "\n";
    }
    return out;
}
} // namespace IR
