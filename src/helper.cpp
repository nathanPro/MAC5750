#include "helper.h"

namespace helper
{
memory_layout::common_t
memory_layout::smooth(std::vector<AST::VarDecl> const& vars)
{
    std::vector<AST::FormalDecl> ans;
    for (auto const& var : vars) {
        auto const& vdr = Grammar::get<AST::VarDeclRule>(var);
        ans.push_back({vdr.type, vdr.name});
    }
    return ans;
}

memory_layout::common_t
memory_layout::smooth(AST::FormalList const& lst)
{
    auto flr = Grammar::get<AST::FormalListRule>(lst);
    return flr.decls;
}

memory_layout::common_t
memory_layout::smooth(AST::ClassDeclNoInheritance const& cls)
{
    return memory_layout::smooth(cls.variables);
}

memory_layout::common_t
memory_layout::smooth(AST::ClassDeclInheritance const& cls)
{
    auto aux = memory_layout::smooth(cls.variables);
    aux.push_back({AST::classType{cls.superclass}, "__base"});
    std::rotate(begin(aux), prev(end(aux)), end(aux));
    return aux;
}

memory_layout::common_t
memory_layout::smooth(AST::MainClassRule const&)
{
    return {};
}

memory_layout::memory_layout() : size(0) {}
memory_layout::memory_layout(meta_data const&               data,
                             std::map<std::string, kind_t>& kind,
                             memory_layout::common_t const& vars)
    : size(0)
{
    for (auto const& var : vars) {
        kind[var.name]  = kind_t::var;
        value[var.name] = size;
        size += data.type_size(var.type);
    }
} // namespace helper

int memory_layout::operator[](std::string const& name) const
{
    return value.at(name);
}

method_spec::method_spec() {}
method_spec::method_spec(memory_layout&& l) : layout(std::move(l)) {}
method_spec::method_spec(memory_layout const& l) : layout(l) {}

void class_spec::init_methods(
    std::vector<AST::MethodDecl> const& mtds)
{
    for (auto const& mtd : mtds) {
        auto const& mdr = Grammar::get<AST::MethodDeclRule>(mtd);
        kind[mdr.name]  = kind_t::method;
        method.insert(
            {mdr.name,
             method_spec{memory_layout{
                 data, kind, memory_layout::smooth(mdr.variables)}}});
    }
}

class_spec::class_spec(meta_data const&          d,
                       AST::MainClassRule const& cls)
    : data(d), base(-1),
      layout(data, kind, memory_layout::smooth(cls))
{
    kind["main"] = kind_t::method;
    method.insert(
        {std::string{"main"}, memory_layout{data, kind, {}}});
}

class_spec::class_spec(meta_data const&                   d,
                       AST::ClassDeclNoInheritance const& cls)
    : data(d), base(-1),
      layout(data, kind, memory_layout::smooth(cls))
{
    init_methods(cls.methods);
}

class_spec::class_spec(meta_data const&                 d,
                       AST::ClassDeclInheritance const& cls)
    : data(d), base(data.c_id.at(cls.superclass)),
      layout(data, kind, memory_layout::smooth(cls))
{
    init_methods(cls.methods);
    int b = base;
    while (b != -1) {
        for (auto const& mtd : data.c_info[b].method)
            method.insert(mtd);
        b = data.c_info[b].base;
    }
}

int class_spec::size() const { return layout.size; }

kind_t class_spec::operator[](std::string const& name) const
{
    if (kind.count(name)) return kind.at(name);
    if (base != -1) return data.c_info[base][name];
    return kind_t::notfound;
}

meta_data::meta_data(AST::Program const& prog)
{
    Grammar::visit(*this, prog);
}

void meta_data::operator()(AST::ProgramRule const& prog)
{
    class_graph top_sort(prog);
    for (int i : top_sort.ans)
        if (i == 0)
            Grammar::visit(*this, prog.main);
        else
            Grammar::visit(*this, prog.classes[i - 1]);
}

void meta_data::operator()(AST::MainClassRule const& cls)
{
    c_id[cls.name] = static_cast<int>(c_info.size());
    c_info.emplace_back(*this, cls);
}

void meta_data::operator()(AST::ClassDeclNoInheritance const& cls)
{
    c_id[cls.name] = static_cast<int>(c_info.size());
    c_info.emplace_back(*this, cls);
}

void meta_data::operator()(AST::ClassDeclInheritance const& cls)
{
    c_id[cls.name] = static_cast<int>(c_info.size());
    c_info.emplace_back(*this, cls);
}

int meta_data::count(std::string const& name) const
{
    return c_id.count(name);
}

class_spec& meta_data::operator[](std::string const& name)
{
    return c_info.at(c_id.at(name));
}

class_spec const& meta_data::operator[](std::string const& name) const
{
    return c_info.at(c_id.at(name));
}

int meta_data::type_size(std::string const& type) const
{
    return (*this)[type].size();
}

int meta_data::type_size(AST::Type const& type) const
{
    struct Visitor {
        meta_data const& data;
        int operator()(AST::integerArrayType const&) { return 8; }
        int operator()(AST::booleanType const&) { return 8; }
        int operator()(AST::integerType const&) { return 8; }
        int operator()(AST::classType const& ct)
        {
            return data.type_size(ct.value);
        }
    };
    return Grammar::visit(Visitor{*this}, type);
}

} // namespace helper
