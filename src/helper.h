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
class meta_data
{
    int class_cnt;

  public:
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

    std::map<std::string, int> classes;
};
} // namespace helper

#endif
