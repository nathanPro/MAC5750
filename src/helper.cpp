#include "helper.h"

namespace helper
{

meta_data::meta_data(const AST::Program& prog) : class_cnt(0)
{
    Grammar::visit(*this, prog);
}

void meta_data::operator()(const AST::ProgramRule& prog)
{
    Grammar::visit(*this, prog.main);
    for (auto& const cls : prog.classes) Grammar::visit(*this, cls);
}

void meta_data::operator()(const AST::MainClassRule& cls)
{
    classes[cls.name] = class_cnt++;
}

void meta_data::operator()(const AST::ClassDeclNoInheritance& cls)
{
    classes[cls.name] = class_cnt++;
}

void meta_data::operator()(const AST::ClassDeclInheritance& cls)
{
    classes[cls.name] = class_cnt++;
}

} // namespace helper
