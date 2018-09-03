#include "lexer.h"
#include "util.h"

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

Lexeme Trie::search(const char* q) const {
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

int main() {
    std::string word;
    Trie reserved(reserved_words);
    while (std::cin >> word)
        write(std::cout, word,
              static_cast<int>(reserved.search(word.c_str())));
}
