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
    std::vector<int> fs(methods.size());
    for (auto const& mtd : methods) v.push_back(mtd.second.stack.sp);
    v.push_back(pos.size());
    std::sort(begin(v), end(v));

    for (auto& mtd : methods) {
        int sp = mtd.second.stack.sp;
        int i  = std::distance(begin(v),
                              lower_bound(begin(v), end(v), sp));
        fs[i]  = mtd.second.stack.spill_size;
    }

    std::vector<int> max_id(v.size());
    for (int i = 0; i < static_cast<int>(pos.size()); i++)
        if (kind[i] == static_cast<int>(IRTag::TEMP)) {
            auto it     = std::prev(upper_bound(begin(v), end(v), i));
            int  j      = *it;
            int  _id    = std::distance(begin(v), it);
            max_id[_id] = std::max(max_id[_id], get_temp(i).id);

            if (i == j) continue;

            int cte = pos.size();
            kind.push_back(static_cast<int>(IRTag::CONST));
            pos.push_back(_const.size());
            _const.push_back(Const{
                fs[j] + 8 * (get_temp(i).id - (get_temp(j).id + 1))});

            int binop = pos.size();
            kind.push_back(static_cast<int>(IRTag::BINOP));
            pos.push_back(_binop.size());
            _binop.push_back(Binop{BinopId::PLUS, j, cte});

            kind[i] = static_cast<int>(IRTag::MEM);
            pos[i]  = _mem.size();
            _mem.push_back(Mem{binop});
        }

    for (auto& mtd : methods) {
        int sp = mtd.second.stack.sp;
        int i  = std::distance(begin(v),
                              lower_bound(begin(v), end(v), sp));
        mtd.second.stack.spill_size +=
            8 * (max_id[i] - get_temp(sp).id);
    }
}

void Tree::fix_registers(int k)
{
    base_register = pos.size();
    for (int i = 0; i < k; i++) {
        kind.push_back(static_cast<int>(IRTag::REG));
        pos.push_back(_reg.size());
        _reg.push_back(Reg{i});
    }
}

int Tree::get_register(int i) { return base_register + i; }

int is_exp(IRTag tag)
{
    return static_cast<int>(tag) < static_cast<int>(IRTag::MOVE);
}

size_t fragment::size() const { return stms.size(); }

std::ostream& operator<<(std::ostream& out, Tree& t)
{
    Catamorphism<ShallowFormat, std::string> F(t);
    tree_dump(out, t, F);
    return out;
}

void Tree::dump(std::ostream& out) const
{
    Catamorphism<DeepFormat, std::string> F(*this);
    tree_dump(out, *this, F);
}

void Tree::emit(int inst)
{
    if (!stm_seq.size() || stm_seq.back() != inst)
        stm_seq.push_back(inst);
}
} // namespace IR
