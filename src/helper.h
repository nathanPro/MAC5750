#ifndef HELPER_BCC
#define HELPER_BCC
#include "AST.h"
#include "grammar.h"
#include "parser.h"
#include "util.h"
#include <map>
#include <string>

namespace helper
{

enum class kind_t { var, instance, method };
class meta_data;

class class_spec
{
    int                           layout_cnt;
    int                           method_cnt;
    std::map<std::string, kind_t> kind;
    int                           __size;
    meta_data&                    data;

  public:
    class_spec(meta_data&, const AST::MainClassRule&);
    class_spec(meta_data&, const AST::ClassDeclNoInheritance&);
    class_spec(meta_data&, const AST::ClassDeclInheritance&);

    int       size() const;
    kind_t    operator[](const std::string&) const;
    int const base;

    std::map<std::string, int> layout;
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
    int               type_size(const AST::Type&) const;
};
} // namespace helper

#endif
