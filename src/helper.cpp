#include "helper.h"

namespace helper
{

class_spec::class_spec(meta_data const& d, const AST::MainClassRule&)
    : method_cnt(0), data(d), base(-1)
{
    kind["main"]   = kind_t::method;
    method["main"] = method_cnt++;
    __size         = 0;
}

void class_spec::init_vars(std::vector<AST::VarDecl> const& vars)
{
    int tot = 0;
    for (const auto& var : vars) {
        auto const& vdr  = Grammar::get<AST::VarDeclRule>(var);
        kind[vdr.name]   = kind_t::var;
        layout[vdr.name] = tot;
        tot += data.type_size(vdr.type);
    }
    __size = tot;
}

void class_spec::init_methods(
    std::vector<AST::MethodDecl> const& mtds)
{
    for (auto const& mtd : mtds) {
        auto const& mdr  = Grammar::get<AST::MethodDeclRule>(mtd);
        kind[mdr.name]   = kind_t::method;
        method[mdr.name] = method_cnt++;
    }
}

class_spec::class_spec(meta_data const&                   d,
                       const AST::ClassDeclNoInheritance& cls)
    : method_cnt(0), data(d), base(-1)
{
    init_vars(cls.variables);
    init_methods(cls.methods);
}

class_spec::class_spec(meta_data const&                 d,
                       const AST::ClassDeclInheritance& cls)
    : method_cnt(0), data(d), base(data.c_id.at(cls.superclass))
{
    init_vars(cls.variables);
    init_methods(cls.methods);
}

kind_t class_spec::operator[](std::string const& name) const
{
    if (kind.count(name)) return kind.at(name);
    return kind_t::notfound;
}

int class_spec::size() const { return __size; }

meta_data::meta_data(const AST::Program& prog)
{
    Grammar::visit(*this, prog);
}

void meta_data::operator()(const AST::ProgramRule& prog)
{
    Grammar::visit(*this, prog.main);
    for (auto const& cls : prog.classes) Grammar::visit(*this, cls);
}

void meta_data::operator()(const AST::MainClassRule& cls)
{
    c_id[cls.name] = static_cast<int>(c_info.size());
    c_info.emplace_back(*this, cls);
}

void meta_data::operator()(const AST::ClassDeclNoInheritance& cls)
{
    c_id[cls.name] = static_cast<int>(c_info.size());
    c_info.emplace_back(*this, cls);
}

void meta_data::operator()(const AST::ClassDeclInheritance& cls)
{
    c_id[cls.name] = static_cast<int>(c_info.size());
    c_info.emplace_back(*this, cls);
}

class_spec& meta_data::operator[](std::string const& name)
{
    return c_info.at(c_id.at(name));
}

class_spec const& meta_data::operator[](std::string const& name) const
{
    return c_info.at(c_id.at(name));
}

int meta_data::count(std::string const& name) const
{
    return c_id.count(name);
}

int meta_data::type_size(const AST::Type& type) const
{
    struct Visitor {
        const meta_data& data;
        int operator()(AST::integerArrayType const&) { return 8; }
        int operator()(AST::booleanType const&) { return 1; }
        int operator()(AST::integerType const&) { return 8; }
        int operator()(AST::classType const& ct)
        {
            return data[ct.value].size();
        }
    };
    return Grammar::visit(Visitor{*this}, type);
}

} // namespace helper
