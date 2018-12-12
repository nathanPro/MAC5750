#ifndef HELPER_BCC
#define HELPER_BCC
#include "AST.h"
#include "class_graph.h"
#include "parser.h"
#include <map>
#include <string>

namespace helper
{

enum class kind_t { notfound, var, instance, method };
class meta_data;

struct memory_layout {
    std::map<std::string, int> value;
    int                        size;

    memory_layout(meta_data const&, std::map<std::string, kind_t>&,
                  std::string const&,
                  std::vector<AST::VarDecl> const&);
    int operator[](std::string const&) const;

  private:
    std::vector<AST::FormalDecl>
    smooth(std::vector<AST::VarDecl> const&);
};

class class_spec
{
    int                           method_cnt;
    std::map<std::string, kind_t> kind;
    meta_data const&              data;

    void init_methods(std::vector<AST::MethodDecl> const&);

  public:
    class_spec(meta_data const&, AST::MainClassRule const&);
    class_spec(meta_data const&, AST::ClassDeclNoInheritance const&);
    class_spec(meta_data const&, AST::ClassDeclInheritance const&);

    int    size() const;
    kind_t operator[](const std::string&) const;
    int    base;

    memory_layout const        layout;
    std::map<std::string, int> method;
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
    int               type_size(std::string const&) const;
    int               type_size(AST::Type const&) const;
};
} // namespace helper

#endif
