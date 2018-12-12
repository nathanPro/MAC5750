#include "class_graph.h"

namespace helper
{

class_graph::class_graph(AST::ProgramRule const& prog) : cnt(0)
{
    {
        auto nc = class_graph::name_collector{*this};
        nc(prog);
    }
    n = names.size();
    G.resize(n);
    seen.resize(n);
    finished.resize(n);
    ans.reserve(n);
    {
        auto ec = class_graph::edge_collector{*this};
        ec(prog);
    }
    for (int i = 0; i < n; i++)
        if (!seen[i]) dfs(i);
    std::reverse(begin(ans), end(ans));
}

void class_graph::dfs(int i)
{
    seen[i] = true;
    for (auto j : G[i]) {
        if (seen[j] && !finished[j])
            throw helper::cyclic_classes{};
        else if (!seen[j])
            dfs(j);
    }
    finished[i] = true;
    ans.push_back(i);
}

void class_graph::name_collector::
     operator()(AST::ProgramRule const& prog)
{
    Grammar::visit(*this, prog.main);
    for (auto const& cls : prog.classes) Grammar::visit(*this, cls);
}

void class_graph::edge_collector::
     operator()(AST::ProgramRule const& prog)
{
    for (auto const& cls : prog.classes) Grammar::visit(*this, cls);
}

void class_graph::edge_collector::init_vars(
    int i, std::vector<AST::VarDecl> const& vars)
{
    for (auto const& var : vars) {
        auto const& vdr = Grammar::get<AST::VarDeclRule>(var);
        if (Grammar::holds<AST::classType>(vdr.type)) {
            std::string const& type_name =
                Grammar::get<AST::classType>(vdr.type).value;
            int const j = cg.names.at(type_name);
            cg.G[j].push_back(i);
        }
    }
}

void class_graph::edge_collector::
     operator()(AST::ClassDeclInheritance const& cls)
{
    int const i = cg.names.at(cls.name);
    int const j = cg.names.at(cls.superclass);
    cg.G[j].push_back(i);
    init_vars(i, cls.variables);
}

void class_graph::edge_collector::
     operator()(AST::ClassDeclNoInheritance const& cls)
{
    init_vars(cg.names.at(cls.name), cls.variables);
}

} // namespace helper
