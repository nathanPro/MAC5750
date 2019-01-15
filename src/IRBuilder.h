#ifndef BCC_IRBUILDER
#define BCC_IRBUILDER

#include "IR.h"

class IRBuilder
{
    IR::Tree&   base;
    int         ref;
    int         kind;
    int         data[5];
    size_t      ds;
    std::string s;

  public:
    IRBuilder(IR::Tree& tree);
    IRBuilder& operator<<(IR::IRTag);
    IRBuilder& operator<<(int);
    IRBuilder& operator<<(std::string);
    int        build();
};

int store_in_temp(IR::Tree&, int);

#endif
