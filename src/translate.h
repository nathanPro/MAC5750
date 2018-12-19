#ifndef BCC_TRANSLATE
#define BCC_TRANSLATE

#include "AST.h"
#include "IR.h"
#include "IRBuilder.h"

namespace IR
{
int translate(Tree&, AST::Exp const&);
int translate(Tree&, AST::Stm const&);
} // namespace IR

#endif
