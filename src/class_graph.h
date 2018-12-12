#include "AST.h"
#include "grammar.h"
#include <algorithm>
#include <map>

namespace helper
{

struct cyclic_classes {
};

class class_graph
{
    int                           cnt;
    int                           n;
    std::vector<std::vector<int>> G;
    std::vector<bool>             seen;
    std::vector<bool>             finished;
    std::map<std::string, int>    names;

    struct name_collector {
        class_graph& cg;

        void operator()(AST::ProgramRule const&);
        template <typename T> void operator()(T const& cls)
        {
            cg.names[cls.name] = cg.cnt++;
        }
    };

    struct edge_collector {
        class_graph& cg;

        void init_vars(int i, std::vector<AST::VarDecl> const&);

        void operator()(AST::ClassDeclNoInheritance const&);
        void operator()(AST::ClassDeclInheritance const&);
        void operator()(AST::ProgramRule const&);
    };

    void dfs(int);

  public:
    std::vector<int> ans;

    class_graph(AST::ProgramRule const&);
};

} // namespace helper
