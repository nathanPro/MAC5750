#include "lexer.h"
#include "util.h"
#include <algorithm>

Trie::Trie(std::vector<std::string>& words) {
    root = std::make_unique<Trie::Node>();
    for (size_t i = 0; i < words.size(); i++) {
        Node* node = root.get();
        for (char c : words[i]) {
            int j = static_cast<int>(c);
            if (!node->child[j])
                node->child[j] = std::make_unique<Trie::Node>();
            node = node->child[j].get();
        }
        node->child[0]        = std::make_unique<Trie::Node>();
        node->child[0]->label = static_cast<Lexeme>(
            static_cast<int>(Lexeme::boolean_keyword) + i);
    }
}

template <typename It> Lexeme Trie::search(It q) const {
    Node* node = root.get();
    while (*q) {
        if (node->child[*q])
            node = node->child[*q++].get();
        else
            return Lexeme::identifier;
    }
    if (!node->child[0]) return Lexeme::identifier;
    return node->child[0]->label;
}

template <typename istream> class Lexer {
    using state = std::pair<Lexeme, std::string>;

    Trie symbols;
    std::string line;
    state curr;
    istream& in;
    size_t lo;
    bool fail = false;

    template <typename F> size_t consume(F f) {
        size_t hi = lo;
        while (hi < line.size() && f(line[hi])) hi++;
        return hi;
    }

    state advance() {
        while (true) {
            if (lo == line.size()) {
                std::getline(in, line);
                if (!in.good()) {
                    fail = true;
                    return {Lexeme::identifier, std::string()};
                }
                lo = 0;
            }
            while (lo < line.size()) {
                while (lo < line.size() && isspace(line[lo])) lo++;

                if (ispunct(line[lo])) {
                    std::string word(line, lo, 2);
                    auto lex = symbols.search(word.c_str());
                    if (lex == Lexeme::identifier) {
                        word = std::string(line, lo, 1);
                        lex  = symbols.search(word.c_str());
                    } else if (lex == Lexeme::inline_comment) {
                        lo = line.size();
                        continue;
                    }
                    lo += word.size();
                    return {lex, word};
                }

                if (isdigit(line[lo])) {
                    size_t hi =
                        consume([](char c) { return isdigit(c); });
                    std::string word(line, lo, hi - lo);
                    lo += word.size();
                    return {Lexeme::integer_literal, word};
                }

                if (isalpha(line[lo])) {
                    std::string word(line, lo, 18);
                    auto lex = symbols.search(word.c_str());
                    if (lex == Lexeme::println_keyword) {
                        lo += 18;
                        return {lex, word};
                    }

                    size_t hi = consume([](char c) {
                        return isalnum(c) || c == '_';
                    });
                    word      = std::string(line, lo, hi - lo);
                    lex       = symbols.search(word.c_str());
                    lo += word.size();
                    return {lex, word};
                }
                __builtin_unreachable();
            }
        }
    }

  public:
    Lexer(istream& _in) : symbols(reserved_words), in(_in), lo(0) {
        curr = advance();
    }

    bool empty() const { return fail; }
    state operator*() { return curr; }
    Lexer& operator++() {
        curr = advance();
        return *this;
    }
};

int main() {
    for (auto lex = Lexer(std::cin); !lex.empty(); ++lex) {
        auto ans = *lex;
        write(std::cout, int(ans.first), ans.second);
    }
}
