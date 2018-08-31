#include "iostream"

template<typename T>
void write(std::ostream& out, T t) {
    out << t << '\n';
}
template<typename T, typename... Args>
void write(std::ostream& out, T t, Args... args) {
    out << t << ' ';
    write(out, args...);
}
