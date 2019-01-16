#include "codegen.h"

namespace GEN
{
codegen::codegen(std::ostream* _out, IR::Tree const& _tree)
    : out(_out), tree(_tree)
{
    *out << prelude;
}
} // namespace GEN
