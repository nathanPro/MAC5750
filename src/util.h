#ifndef UTIL_BCC
#define UTIL_BCC
#include "iostream"

namespace Util {

template <typename T> void write(std::ostream& out, T t) {
    out << t << '\n';
}

template <typename T, typename... Args>
void write(std::ostream& out, T t, Args... args) {
    out << t << ' ';
    write(out, args...);
}

template <typename... Args> void debug(Args... args) {
#ifdef DEBUG
    write(std::cerr, args...);
#endif
}

template <class... Ts> struct type_switch : Ts... {
    using Ts::operator()...;
};
template <class... Ts> type_switch(Ts...)->type_switch<Ts...>;

} // namespace Util
#endif
