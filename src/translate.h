#ifndef BCC_TRANSLATE
#define BCC_TRANSLATE

#include "AST.h"
#include "IR.h"
#include "IRBuilder.h"

namespace IR
{
int translate(Tree& dst, AST::Exp const& exp);
} // namespace IR

#endif
