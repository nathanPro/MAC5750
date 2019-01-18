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
    }                                                                \
    ret const& Tree::get##name(int ref) const                        \
    {                                                                \
        check_id(kind[ref], static_cast<int>(ID));                   \
        return name[pos[ref]];                                       \
    }

IR_GETTER(_const, IRTag::CONST, Const)
IR_GETTER(_reg, IRTag::REG, Reg)
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
IR_GETTER(_push, IRTag::PUSH, Push)
IR_GETTER(_pop, IRTag::POP, Pop)

#undef IR_GETTER

Tree::Tree() : tmp(0), lbl(0) {}

size_t Tree::size() const { return pos.size(); }

IRTag Tree::get_type(int ref) const
{
    return static_cast<IRTag>(kind[ref]);
}

Explist Tree::get_explist(int ref) const { return _explist[ref]; }

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

void Tree::simplify()
{
    spill();
    mark_sp();
}

void Tree::mark_sp()
{
    for (auto const& mtd : methods) {
        int sp   = mtd.second.stack.sp;
        kind[sp] = static_cast<int>(IRTag::REG);
        pos[sp]  = _reg.size();
        _reg.push_back(Reg{0});
    }
}

void Tree::spill()
{
    std::vector<int> v;
    for (auto const& mtd : methods) v.push_back(mtd.second.stack.sp);
    v.push_back(pos.size());
    std::sort(begin(v), end(v));
    for (int i = 0; i < static_cast<int>(pos.size()); i++)
        if (kind[i] == static_cast<int>(IRTag::TEMP)) {
            int j = *std::prev(upper_bound(begin(v), end(v), i));
            if (i == j) continue;

            int cte = pos.size();
            kind.push_back(static_cast<int>(IRTag::CONST));
            pos.push_back(_const.size());
            _const.push_back(
                Const{8 * (get_temp(i).id - (get_temp(j).id + 1))});

            int binop = pos.size();
            kind.push_back(static_cast<int>(IRTag::BINOP));
            pos.push_back(_binop.size());
            _binop.push_back(Binop{BinopId::PLUS, j, cte});

            kind[i] = static_cast<int>(IRTag::MEM);
            pos[i]  = _mem.size();
            _mem.push_back(Mem{binop});
        }
}

size_t fragment::size() const { return stms.size(); }

template <typename C> struct Format {

    std::string operator()(Const const& c)
    {
        return std::string("CONST ") + std::to_string(c.value);
    }
    std::string operator()(Reg const& r)
    {
        return std::string("REG ") + std::to_string(r.id);
    }
    std::string operator()(Temp const& t)
    {
        return std::string("TEMP ") + std::to_string(t.id);
    }
    std::string operator()(Binop const& b)
    {
        static std::vector<std::string> names = {
            "+", "-", "*", "/", "&", "|", "<<", ">>", "ARSHIFT", "^"};
        return std::string("BINOP ") + names[b.op] +
               std::string(": ") + std::to_string(b.lhs) +
               std::string(" ") + std::to_string(b.rhs);
    }
    std::string operator()(Mem const& m)
    {
        return std::string("MEM ") + std::to_string(m.exp);
    }
    std::string operator()(Call const& c)
    {
        return std::string("CALL") + c.fn + std::string(" ") +
               std::to_string(c.explist);
    }
    std::string operator()(Cmp const& c)
    {
        return std::string("CMP ") + std::to_string(c.lhs) +
               std::string(" ") + std::to_string(c.rhs);
    }
    std::string operator()(Move const& m)
    {
        return std::string("MOVE ") + std::to_string(m.dst) +
               std::string(" ") + std::to_string(m.src);
    }
    std::string operator()(Exp const& e)
    {
        return std::string("EXP ") + std::to_string(e.exp);
    }
    std::string operator()(Jmp const& j)
    {
        return std::string("JMP ") + std::to_string(j.target);
    }
    std::string operator()(Cjmp const& c)
    {
        return std::string("CJMP ") + std::to_string(c.temp) +
               std::string(" ") + std::to_string(c.target);
    }
    std::string operator()(Label const& l)
    {
        return std::string("LABEL ") + std::to_string(l.id);
    }
    std::string operator()(Push const& p)
    {
        return std::string("PUSH ") + std::to_string(p.ref);
    }
    std::string operator()(Pop const& p)
    {
        return std::string("POP ") + std::to_string(p.ref);
    }

    Format(C&& __fmap) : fmap(__fmap) {}
    C fmap;
};

std::ostream& operator<<(std::ostream& out, Tree& t)
{
    IR::Catamorphism<Format, std::string> F(t);

    Util::write(out, "Tree has", t.pos.size(), "nodes");
    for (int i = 0; i < static_cast<int>(t.kind.size()); i++)
        Util::write(out, "\t", i, ":\t", F(i));

    Util::write(out, "\nIt has", t.methods.size(),
                "function fragments");
    for (auto const& f : t.methods) {
        Util::write(out, f.first);
        if (t.aliases[f.first].empty())
            Util::write(out, f.first, "has no aliases");
        else
            for (auto const& a : t.aliases[f.first])
                Util::write(out, a, "is an alias of it");

        Util::write(out, "The arguments are:");
        int sp = f.second.stack.sp, tp = f.second.stack.tp;
        Util::write(out, "\t", "sp", ":=", "[", sp, "]\t", F(sp));
        Util::write(out, "\t", "tp", ":=", "[", tp, "]\t", F(tp));
        for (auto const& a : f.second.stack.arguments)
            Util::write(out, "\t", a.first, ":=", "[", a.second, "]",
                        "\t", F(a.second));
        Util::write(out, "The code is:");
        for (auto const& s : f.second.stms)
            Util::write(out, "\t", s, ":\t", F(s));
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
