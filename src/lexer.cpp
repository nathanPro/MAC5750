#include "lexer.h"
#include "util.h"
#include <algorithm>

Trie::Trie(const std::vector<std::string>& words) {
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
