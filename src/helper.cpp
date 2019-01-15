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
    return memory_layout::smooth(cls.variables);
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
    : size(0), source([&] {
          common_t ans;
          for (auto const& var : vars)
              ans.push_back({var.type, var.name});
          return ans;
      }())
{
    for (auto const& [type, name] : source) {
        kind[name]  = kind_t::var;
        value[name] = size;
        size += data.type_size(type);
    }
}

bool memory_layout::has(std::string const& name) const
{
    return value.count(name) != 0;
}

int memory_layout::operator[](std::string const& name) const
{
    return value.at(name);
}

method_spec::method_spec(meta_data const& d, class_spec const& c,
                         std::string const& n, memory_layout&& l,
                         memory_layout::common_t&& _arglist)
    : data(d), cls(c), name(n), layout(std::move(l)),
      arglist(std::move(_arglist))
{
}
method_spec::method_spec(meta_data const& d, class_spec const& c,
                         std::string const& n, memory_layout const& l,
                         memory_layout::common_t&& _arglist)
    : data(d), cls(c), name(n), layout(l),
      arglist(std::move(_arglist))
{
}

void class_spec::insert_method(std::string const&        name,
                               memory_layout&&           layout,
                               memory_layout::common_t&& args)
{
    if (m_id.count(name)) return;
    kind[name] = kind_t::method;
    m_id[name] = m_info.size();
    m_info.emplace_back(data, *this, name, std::move(layout),
                        std::move(args));
}

void class_spec::insert_method(std::string const& name,
                               method_spec const& mtd)
{
    if (m_id.count(name)) return;
    kind[name] = kind_t::method;
    m_id[name] = m_info.size();
    m_info.push_back(mtd);
}

void class_spec::init_methods(
    std::vector<AST::MethodDecl> const& mtds)
{
    for (auto const& mtd : mtds) {
        auto const& mdr = Grammar::get<AST::MethodDeclRule>(mtd);
        insert_method(
            mdr.name,
            memory_layout{data, kind,
                          memory_layout::smooth(mdr.variables)},
            memory_layout::smooth(mdr.arguments));
    }
}

class_spec::class_spec(meta_data const&          d,
                       AST::MainClassRule const& cls)
    : data(d), name(cls.name), base(-1),
      variable(data, kind, memory_layout::smooth(cls))
{
    insert_method("main", memory_layout{data, kind, {}}, {});
}

class_spec::class_spec(meta_data const&                   d,
                       AST::ClassDeclNoInheritance const& cls)
    : data(d), name(cls.name), base(-1),
      variable(data, kind, memory_layout::smooth(cls))
{
    init_methods(cls.methods);
}

class_spec::class_spec(meta_data const&                 d,
                       AST::ClassDeclInheritance const& cls)
    : data(d), name(cls.name), base(data.c_id.at(cls.superclass)),
      variable([&] {
          auto ans = data[cls.superclass].variable;
          for (auto const& var :
               memory_layout::smooth(cls.variables)) {
              ans.value.insert({var.name, ans.size});
              ans.size += data.type_size(var.type);
          }
          return ans;
      }())
{
    init_methods(cls.methods);
    for (int b = base; b != -1; b = data.c_info[b].base) {
        auto const& info = data.c_info[b];
        for (auto const& i : info.m_id)
            insert_method(i.first, info.m_info[i.second]);
        kind.insert(begin(info.kind), end(info.kind));
    }
}

int class_spec::size() const { return variable.size; }

kind_t class_spec::operator[](std::string const& name) const
{
    if (kind.count(name)) return kind.at(name);
    if (base != -1) return data.c_info[base][name];
    return kind_t::notfound;
}

method_spec& class_spec::method(std::string const& name)
{
    return m_info[m_id.at(name)];
}

method_spec const& class_spec::method(std::string const& name) const
{
    return m_info[m_id.at(name)];
}

meta_data::meta_data(AST::Program const& prog)
{
    Grammar::visit(*this, prog);
}

meta_data::meta_data() {}

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

std::string mangle(std::string cls, std::string mtd)
{
    return std::string("_Z") + cls + std::string("_") + mtd;
}

} // namespace helper
