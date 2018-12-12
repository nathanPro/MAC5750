#ifndef HELPER_BCC
#define HELPER_BCC
#include "AST.h"
#include "grammar.h"
#include "parser.h"
#include "util.h"
#include <algorithm>
#include <map>
#include <string>

namespace helper
{

struct cyclic_classes {
};

enum class kind_t { notfound, var, instance, method };
class meta_data;

class class_spec
{
    int                           method_cnt;
    std::map<std::string, kind_t> kind;
    int                           __size;
    meta_data const&              data;

    void init_vars(std::vector<AST::VarDecl> const&);
    void init_methods(std::vector<AST::MethodDecl> const&);

  public:
    class_spec(meta_data const&, const AST::MainClassRule&);
    class_spec(meta_data const&, const AST::ClassDeclNoInheritance&);
    class_spec(meta_data const&, const AST::ClassDeclInheritance&);

    int       size() const;
    kind_t    operator[](const std::string&) const;
    int const base;

    std::map<std::string, int> layout;
    std::map<std::string, int> method;
};

class class_graph
{
    int                           cnt;
    int                           n;
    std::vector<std::vector<int>> G;
    std::vector<bool>             seen;
    std::vector<bool>             finished;

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
    std::map<std::string, int> names;
    std::vector<int>           ans;

    class_graph(AST::ProgramRule const&);
};

class meta_data
{
    std::vector<class_spec>    c_info;
    std::map<std::string, int> c_id;

  public:
    friend class class_spec;
    meta_data(const AST::Program&);

    void operator()(const AST::integerArrayType&);
    void operator()(const AST::booleanType&);
    void operator()(const AST::integerType&);
    void operator()(const AST::classType&);
    void operator()(const AST::VarDeclRule&);
    void operator()(const AST::MethodDeclRule&);
    void operator()(const AST::ClassDeclNoInheritance&);
    void operator()(const AST::ClassDeclInheritance&);
    void operator()(const AST::MainClassRule&);
    void operator()(const AST::ProgramRule&);

    int               count(std::string const&) const;
    class_spec&       operator[](std::string const&);
    class_spec const& operator[](std::string const&) const;
    int               type_size(const AST::Type&) const;
};
} // namespace helper

#endif
