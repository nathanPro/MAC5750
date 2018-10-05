#ifndef BCC_IRBUILDER
#define BCC_IRBUILDER

#include "IR.h"

class IRBuilder {
    IR::Tree& base;
    int       ref;
    int       kind;
    int       data[5];
    size_t    ds;

  public:
    IRBuilder(IR::Tree& tree);
    IRBuilder& operator<<(IR::IRTag in);
    IRBuilder& operator<<(int in);
    int        build();
};

#endif
