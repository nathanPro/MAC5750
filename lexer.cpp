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

void found(Lexeme lex) {
    int i = static_cast<int>(lex);
    if (i == 0)
        write(std::cout, "FOUND!", i, "identifier");
    else if (i == 1)
        write(std::cout, "FOUND!", i, "integer constant");
    else
        write(std::cout, "FOUND!", i, reserved_words[i - 2]);
}

template <typename F>
std::pair<Lexeme, std::string> match(Trie& trie, std::string& line,
                                     size_t& lo, F f) {
    std::string word;
    size_t hi = lo;
    while (hi < line.size() && f(line[hi]))
        word.push_back(line[hi++]);
    lo = hi;
    return {trie.search(word.c_str()), word};
}

int main() {
    std::string line;
    Trie reserved(reserved_words);
    while (std::getline(std::cin, line)) {
        write(std::cout, "Line:", line);
        size_t lo = 0;
        while (lo < line.size()) {
            while (lo < line.size() && isspace(line[lo])) lo++;

            write(std::cerr, lo, line.size());
            {
                Lexeme ans = reserved.search(line.begin() + lo);
                if (ans == Lexeme::println_keyword) {
                    lo += 18;
                    found(ans);
                    continue;
                }
            }

            if (isalpha(line[lo])) {
                auto ans = match(reserved, line, lo, [](char c) {
                    return isalpha(c) || isdigit(c) || c == '_';
                });
                write(std::cout, int(ans.first), ans.second);
                continue;
            }

            if (isdigit(line[lo])) {
                auto ans  = match(reserved, line, lo,
                                 [](char c) { return isdigit(c); });
                ans.first = Lexeme::integer_literal;
                write(std::cout, int(ans.first), ans.second);
                continue;
            }

            // TODO fix punct
            if (ispunct(line[lo])) {
                auto ans  = match(reserved, line, lo,
                                 [](char c) { return ispunct(c); });
                ans.first = Lexeme::integer_literal;
                write(std::cout, int(ans.first), ans.second);
                continue;
            }

            write(std::cerr, "Error lexing");
            return 1;
        }
    }
}
