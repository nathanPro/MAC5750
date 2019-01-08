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
class class_spec;

struct memory_layout {
    using common_t = std::vector<AST::FormalDecl>;
    std::map<std::string, int> value;
    int                        size;

    memory_layout();
    memory_layout(meta_data const&, std::map<std::string, kind_t>&,
                  common_t const&);
    int operator[](std::string const&) const;

    static common_t smooth(std::vector<AST::VarDecl> const&);
    static common_t smooth(AST::FormalList const&);
    static common_t smooth(AST::ClassDeclNoInheritance const&);
    static common_t smooth(AST::ClassDeclInheritance const&);
    static common_t smooth(AST::MainClassRule const&);
};

class method_spec
{
    meta_data const&  data;
    class_spec const& cls;

  public:
    method_spec(meta_data const&, class_spec const&,
                std::string const&, memory_layout&&);
    method_spec(meta_data const&, class_spec const&,
                std::string const&, memory_layout const&);

    std::string const   name;
    memory_layout const layout;
};

class class_spec
{
    meta_data const&              data;
    std::map<std::string, kind_t> kind;
    std::map<std::string, int>    m_id;
    std::vector<method_spec>      m_info;

    void init_methods(std::vector<AST::MethodDecl> const&);
    void insert_method(std::string const&, memory_layout&&);
    void insert_method(std::string const&, method_spec const&);

  public:
    class_spec(meta_data const&, AST::MainClassRule const&);
    class_spec(meta_data const&, AST::ClassDeclNoInheritance const&);
    class_spec(meta_data const&, AST::ClassDeclInheritance const&);

    int    size() const;
    kind_t operator[](const std::string&) const;

    std::string const&  name;
    int                 base;
    memory_layout const variable;
    method_spec&        method(std::string const&);
    method_spec const&  method(std::string const&) const;
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
