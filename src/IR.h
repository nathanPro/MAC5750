#ifndef BCC_IR
#define BCC_IR

#include <vector>
#include <cassert>

class IRBuilder;
namespace IR {

enum ExpId
{
    CONST,
    NAME,
    TEMP,
    BINOP,
    MEM,
    CALL,
    ESEQ
};

struct Const {
    int value;
};

struct Name {
    int label;
};

class Tree {
    friend class ::IRBuilder;
    size_t id;
    std::vector<int> kind;
    std::vector<int> pos;
    std::vector<Const> _const;
    std::vector<Name> _name;

  public:
    Tree() : id(0) {}
    Const& get_const(int ref);
    Name& get_name(int ref);
    int get_type(int ref);
    size_t size() const;
};
}

#endif
