#ifndef BCC_IRBUILDER
#define BCC_IRBUILDER

#include "IR.h"

class IRBuilder {
    IR::Tree& base;
    int ref;

  public:
    IRBuilder(IR::Tree& tree);
    operator int();
    IRBuilder& operator<<(IR::ExpId in);
    IRBuilder& operator<<(int in);
};

#endif
