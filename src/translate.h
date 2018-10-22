#ifndef BCC_TRANSLATE
#define BCC_TRANSLATE

#include "AST.h"
#include "IR.h"

namespace AST {
int translate(IR::Tree& dst, const AST::Exp& exp);
} // namespace AST

#endif
