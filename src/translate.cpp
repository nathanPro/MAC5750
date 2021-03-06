#include "translate.h"

namespace IR
{

Translator::Translator(Tree& tree) : t(tree) {}
Translator::Translator(Tree& tree, helper::meta_data&& d)
    : t(tree), data(std::move(d))
{
}

int Translator::binop(BinopId                                    id,
                      AST::__detail::BinaryRule<AST::Exp> const& exp)
{
    IRBuilder ans(t);
    ans << IRTag::BINOP << id << Grammar::visit(*this, exp.lhs)
        << Grammar::visit(*this, exp.rhs);
    return ans.build();
}

int Translator::operator()(AST::andExp const& exp)
{
    return binop(IR::BinopId::AND, exp);
}

int Translator::operator()(AST::sumExp const& exp)
{
    return binop(IR::BinopId::PLUS, exp);
}

int Translator::operator()(AST::minusExp const& exp)
{
    return binop(IR::BinopId::MINUS, exp);
}

int Translator::operator()(AST::prodExp const& exp)
{
    return binop(IR::BinopId::MUL, exp);
}

int Translator::operator()(AST::integerExp const& exp)
{
    IRBuilder builder(t);
    builder << IRTag::CONST << exp.value;
    return builder.build();
}

int Translator::operator()(AST::trueExp const&)
{
    IRBuilder builder(t);
    builder << IRTag::CONST << 0x80;
    return builder.build();
}

int Translator::operator()(AST::falseExp const&)
{
    IRBuilder builder(t);
    builder << IRTag::CONST << 0;
    return builder.build();
}

int Translator::operator()(AST::parenExp const& exp)
{
    return Grammar::visit(*this, exp.inner);
}

int Translator::operator()(AST::lessExp const& exp)
{

    IRBuilder bn(t);

    bn << IR::IRTag::BINOP << IR::BinopId::AND
       << store_in_temp(t,
                        [&] {
                            IRBuilder cmp(t);
                            cmp << IRTag::CMP
                                << Grammar::visit(*this, exp.lhs)
                                << Grammar::visit(*this, exp.rhs);
                            return cmp.build();
                        }())
       << Grammar::visit(*this, AST::Exp{AST::trueExp{}});

    return bn.build();
}

int Translator::operator()(AST::bangExp const& exp)
{
    IRBuilder bn(t);

    bn << IR::IRTag::BINOP << IR::BinopId::XOR
       << Grammar::visit(*this, exp.inner)
       << Grammar::visit(*this, AST::Exp{AST::trueExp{}});

    return bn.build();
}

int Translator::operator()(AST::identifierExp const& exp)
{
    if (frame.arguments.count(exp.value))
        return frame.arguments[exp.value];

    int pt  = -1;
    int dsp = 0;
    if (data[current_class]
            .method(current_method)
            .layout.has(exp.value))
        pt = frame.sp, dsp = data[current_class]
                                 .method(current_method)
                                 .layout[exp.value];
    else if (data[current_class].variable.has(exp.value))
        pt = frame.tp, dsp = data[current_class].variable[exp.value];

    IRBuilder mem(t);
    IRBuilder binop(t);
    binop << IRTag::BINOP << BinopId::PLUS << pt << [&] {
        IRBuilder cte(t);
        cte << IRTag::CONST << dsp;
        return cte.build();
    }();
    mem << IRTag::MEM << binop.build();
    return mem.build();
}

int Translator::operator()(AST::thisExp const&) { return frame.tp; }
int Translator::operator()(AST::methodCallExp const& exp)
{
    auto const& cls_name =
        Grammar::get<AST::classType>(
            Grammar::visit(TypeInferenceVisitor{*this}, exp.object))
            .value;
    auto const& es =
        Grammar::get<AST::ExpListRule>(exp.arguments).exps;

    IRBuilder call(t);
    call << IRTag::CALL << helper::mangle(cls_name, exp.name) << [&] {
        Explist args;
        args.push_back(
            store_in_temp(t, Grammar::visit(*this, exp.object)));
        for (auto const& e : es)
            args.push_back(
                store_in_temp(t, Grammar::visit(*this, e)));
        return t.keep_explist(std::move(args));
    }();

    return store_in_temp(t, call.build());
}

int Translator::operator()(AST::ExpListRule const&) { return -1; }
int Translator::operator()(AST::lengthExp const&) { return -1; }
int Translator::operator()(AST::indexingExp const&) { return -1; }
int Translator::operator()(AST::newArrayExp const&) { return -1; }
int Translator::operator()(AST::blockStm const& blk)
{
    for (auto const& s : blk.statements) Grammar::visit(*this, s);
    return 0;
}

int Translator::operator()(AST::ifStm const& ifs)
{
    auto cnd = store_in_temp(t, Grammar::visit(*this, ifs.condition));
    auto if_lbl  = t.new_label();
    auto end_lbl = t.new_label();
    {
        IRBuilder cjmp(t);
        cjmp << IR::IRTag::CJMP << cnd << int(if_lbl);
        cjmp.build();
    }
    Grammar::visit(*this, ifs.else_clause);
    {
        IRBuilder jmp(t);
        jmp << IR::IRTag::JMP << end_lbl;
        jmp.build();
    }
    t.place_label(std::move(if_lbl));
    Grammar::visit(*this, ifs.if_clause);
    t.place_label(std::move(end_lbl));
    return 0;
}

int Translator::operator()(AST::whileStm const& wst)
{
    auto cnd_lbl = t.place_label(t.new_label());
    auto cnd = store_in_temp(t, Grammar::visit(*this, wst.condition));
    auto bdy_lbl = t.new_label();
    auto out_lbl = t.new_label();
    {
        IRBuilder cjmp(t);
        cjmp << IR::IRTag::CJMP << cnd << int(bdy_lbl);
        cjmp.build();
    }
    {
        IRBuilder jmp(t);
        jmp << IR::IRTag::JMP << int(out_lbl);
        jmp.build();
    }
    t.place_label(std::move(bdy_lbl));
    Grammar::visit(*this, wst.body);
    {
        IRBuilder jmp(t);
        jmp << IR::IRTag::JMP << int(cnd_lbl);
        jmp.build();
    }
    t.place_label(std::move(out_lbl));
    return 0;
}
int Translator::operator()(AST::assignStm const& ast)
{
    auto lhs_exp = AST::Exp{AST::identifierExp{ast.name}};

    auto lhs_t = Grammar::visit(TypeInferenceVisitor{*this}, lhs_exp);
    auto rhs_t =
        Grammar::visit(TypeInferenceVisitor{*this}, ast.value);

    if (Grammar::index(lhs_t) != Grammar::index(rhs_t)) throw;

    auto lhs = Grammar::visit(*this, lhs_exp);
    auto rhs = Grammar::visit(*this, ast.value);

    if (Grammar::holds<AST::classType>(lhs_t)) {
        auto const& cls_name =
            Grammar::get<AST::classType>(lhs_t).value;

        if (t.get_type(lhs) == IR::IRTag::MEM)
            lhs = t.get_mem(lhs).exp;

        Explist args;
        args.push_back(store_in_temp(t, lhs));
        args.push_back(store_in_temp(t, rhs));
        args.push_back([&] {
            IRBuilder cte(t);
            cte << IR::IRTag::CONST << data[cls_name].size();
            return cte.build();
        }());

        auto explist = t.keep_explist(std::move(args));

        IRBuilder exp(t);
        exp << IR::IRTag::EXP << [&] {
            IRBuilder call(t);
            call << IR::IRTag::CALL << std::string("memcpy")
                 << explist;
            return call.build();
        }();
        return exp.build();
    }

    IRBuilder mov(t);
    mov << IR::IRTag::MOVE << lhs << rhs;
    return mov.build();
}

int Translator::operator()(AST::indexAssignStm const&) { return -1; }

int Translator::operator()(AST::printStm const& stm)
{
    IRBuilder exp(t);
    exp << IR::IRTag::EXP << [&] {
        IRBuilder builder(t);

        builder << IR::IRTag::CALL << std::string("print");
        builder << t.keep_explist(Explist{
            store_in_temp(t, Grammar::visit(*this, stm.exp))});
        return builder.build();
    }();
    return exp.build();
}

int Translator::operator()(AST::newObjectExp const& noe)
{
    return store_in_temp(t, [&] {
        IRBuilder call(t);
        IRBuilder cte(t);
        cte << IR::IRTag::CONST << data[noe.value].size();

        call << IR::IRTag::CALL << std::string("malloc")
             << t.keep_explist(Explist{cte.build()});
        return call.build();
    }());
}

fragmentGuard::fragmentGuard(Tree& _t, std::string _l,
                             IR::activation_record _r)
    : t(_t), label(_l), rec(_r)
{
    t.stm_seq = {};
}
fragmentGuard::~fragmentGuard()
{
    t.methods.insert({label, {rec, std::move(t.stm_seq)}});
}

int Translator::operator()(AST::MethodDeclRule const& mdr)
{
    current_method = mdr.name;

    frame = {
        {},
        t.new_temp(),
        t.new_temp(),
        8 * (data[current_class].method(current_method).layout.size)};

    auto flr = Grammar::get<AST::FormalListRule>(mdr.arguments);
    for (auto const& d : flr.decls)
        frame.arguments.insert({d.name, t.new_temp()});

    fragmentGuard guard(
        t, helper::mangle(current_class, current_method), frame);

    for (auto const& stm : mdr.body) Grammar::visit(*this, stm);

    auto ret_t =
        Grammar::visit(TypeInferenceVisitor{*this}, mdr.return_exp);

    IRBuilder ret(t);
    ret << IRTag::EXP << Grammar::visit(*this, mdr.return_exp);
    ret.build();

    return 0;
}
int Translator::operator()(AST::ClassDeclNoInheritance const& cls)
{
    current_class = cls.name;
    for (auto const& mtd : cls.methods) Grammar::visit(*this, mtd);
    return 0;
}
int Translator::operator()(AST::ClassDeclInheritance const& cls)
{
    current_class = cls.name;
    for (auto const& mtd : cls.methods) Grammar::visit(*this, mtd);
    for (auto const& mtd : data[current_class].methods)
        if (data[current_class][mtd] == helper::kind_t::method_inh)
            for (int b = data[current_class].base; b != -1;
                 b     = data[b].base) {
                if (data[b][mtd] == helper::kind_t::method_def) {
                    t.aliases[helper::mangle(data[b].name, mtd)]
                        .insert(helper::mangle(cls.name, mtd));
                    break;
                }
            }
    return 0;
}
int Translator::operator()(AST::MainClassRule const& mc)
{
    fragmentGuard guard(t, std::string("main"),
                        {{}, t.new_temp(), t.new_temp(), 0});

    Grammar::visit(*this, mc.body);

    return 0;
}
int Translator::operator()(AST::ProgramRule const& prog)
{
    Grammar::visit(*this, prog.main);
    for (auto const& c : prog.classes) Grammar::visit(*this, c);
    return 0;
}

int translate(Tree& t, AST::Exp const& exp)
{
    return Grammar::visit(Translator{t}, exp);
}

int translate(Tree& t, AST::Stm const& stm)
{
    return Grammar::visit(Translator{t}, stm);
}

void translate(Tree& t, AST::Program const& p)
{
    Grammar::visit(Translator{t, helper::meta_data(p)}, p);
}

AST::Type TypeInferenceVisitor::
          operator()(AST::methodCallExp const& exp)
{
    auto obj = Grammar::get<AST::classType>(
        Grammar::visit(*this, exp.object));

    int  cnt     = 0;
    auto explist = [&] {
        helper::memory_layout::common_t ans;
        for (auto const& e :
             Grammar::get<AST::ExpListRule>(exp.arguments).exps)
            ans.push_back(
                {Grammar::visit(*this, e), std::to_string(cnt++)});
        return ans;
    }();

    for (int i = 0; i < cnt; i++)
        if (Grammar::index(explist[i].type) !=
            Grammar::index(translator.data[obj.value]
                               .method(exp.name)
                               .arglist[i]
                               .type))
            throw;

    return translator.data[obj.value].method(exp.name).return_type;
}

AST::Type TypeInferenceVisitor::operator()(AST::thisExp const&)
{
    return AST::classType{translator.current_class};
}

AST::Type TypeInferenceVisitor::
          operator()(AST::identifierExp const& exp)
{
    helper::memory_layout::common_t dft;

    auto const& source = [&] {
        if (translator.frame.arguments.count(exp.value))
            return translator.data[translator.current_class]
                .method(translator.current_method)
                .arglist;

        if (translator.data[translator.current_class]
                .method(translator.current_method)
                .layout.has(exp.value))
            return translator.data[translator.current_class]
                .method(translator.current_method)
                .layout.source;

        if (translator.data[translator.current_class].variable.has(
                exp.value))
            return translator.data[translator.current_class]
                .variable.source;
        return dft;
    }();
    if (!source.size()) throw;

    for (auto const& [type, name] : source)
        if (name == exp.value) return type;
    throw;
}

AST::Type TypeInferenceVisitor::operator()(AST::andExp const& exp)
{
    type_assert<AST::booleanType>(Grammar::visit(*this, exp.lhs));
    type_assert<AST::booleanType>(Grammar::visit(*this, exp.rhs));
    return AST::booleanType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::lessExp const& exp)
{
    type_assert<AST::integerType>(Grammar::visit(*this, exp.lhs));
    type_assert<AST::integerType>(Grammar::visit(*this, exp.rhs));
    return AST::booleanType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::sumExp const& exp)
{
    type_assert<AST::integerType>(Grammar::visit(*this, exp.lhs));
    type_assert<AST::integerType>(Grammar::visit(*this, exp.rhs));
    return AST::integerType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::minusExp const& exp)
{
    type_assert<AST::integerType>(Grammar::visit(*this, exp.lhs));
    type_assert<AST::integerType>(Grammar::visit(*this, exp.rhs));
    return AST::integerType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::prodExp const& exp)
{
    type_assert<AST::integerType>(Grammar::visit(*this, exp.lhs));
    type_assert<AST::integerType>(Grammar::visit(*this, exp.rhs));
    return AST::integerType{};
}

AST::Type TypeInferenceVisitor::
          operator()(AST::indexingExp const& exp)
{
    type_assert<AST::integerArrayType>(
        Grammar::visit(*this, exp.lhs));
    type_assert<AST::integerType>(Grammar::visit(*this, exp.rhs));
    return AST::integerType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::lengthExp const& exp)
{
    type_assert<AST::integerArrayType>(
        Grammar::visit(*this, exp.inner));
    return AST::integerType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::integerExp const&)
{
    return AST::integerType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::trueExp const&)
{
    return AST::booleanType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::falseExp const&)
{
    return AST::booleanType{};
}

AST::Type TypeInferenceVisitor::
          operator()(AST::newArrayExp const& exp)
{
    type_assert<AST::integerType>(Grammar::visit(*this, exp.inner));
    return AST::integerArrayType{};
}

AST::Type TypeInferenceVisitor::
          operator()(AST::newObjectExp const& exp)
{
    return AST::classType{exp.value};
}

AST::Type TypeInferenceVisitor::operator()(AST::bangExp const& exp)
{
    type_assert<AST::booleanType>(Grammar::visit(*this, exp.inner));
    return AST::booleanType{};
}

AST::Type TypeInferenceVisitor::operator()(AST::parenExp const& exp)
{
    return AST::Type{Grammar::visit(*this, exp.inner)};
}

} // namespace IR
