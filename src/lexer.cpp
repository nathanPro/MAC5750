#include "lexer.h"

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

Lexer::Lexer(std::istream* _in, size_t _la)
    : symbols(reserved_words), in(_in), lo(0), la(_la), lc(0), pos(0),
      LA(la) {
    for (size_t i = 0; i < la; i++) LA[i] = advance();
}

LexState Lexer::operator[](size_t i) { return LA[(pos + i) % la]; }

LexState Lexer::advance() {
    if (LA[pos].first == Lexeme::eof) return LA[pos];
    while (true) {
        if (lo == line.size()) {
            std::getline(*in, line);
            lo = 0;
            if (!in->good()) return {Lexeme::eof, std::string(), lc};
            lc++;
        }
        while (lo < line.size()) {
            while (lo < line.size() && isspace(line[lo])) lo++;
            if (lo == line.size()) break;

            if (ispunct(line[lo])) {
                // Still need to fix /* */ comment style
                std::string word(line, lo, 2);
                auto        lex = symbols.search(word.c_str());
                if (lex == Lexeme::identifier) {
                    word = std::string(line, lo, 1);
                    lex  = symbols.search(word.c_str());
                } else if (lex == Lexeme::inline_comment) {
                    lo = line.size();
                    continue; // NON-OBVIOUS CONTROL FLOW
                }
                lo += word.size();
                return {lex, word, lc};
            }

            if (isdigit(line[lo])) {
                size_t hi =
                    consume([](char c) { return isdigit(c); });
                std::string word(line, lo, hi - lo);
                lo += word.size();
                return {Lexeme::integer_literal, word, lc};
            }

            if (isalpha(line[lo])) {
                std::string word(line, lo, 18);
                auto        lex = symbols.search(word.c_str());
                if (lex == Lexeme::println_keyword) {
                    lo += 18;
                    return {lex, word, lc};
                }

                size_t hi = consume(
                    [](char c) { return isalnum(c) || c == '_'; });
                word = std::string(line, lo, hi - lo);
                lex  = symbols.search(word.c_str());
                lo += word.size();
                return {lex, word, lc};
            }
            __builtin_unreachable();
        }
    }
}

bool Lexer::empty() const { return LA[pos].first == Lexeme::eof; }

size_t Lexer::line_count() const { return LA[pos].third; }

LexState Lexer::operator*() { return LA[pos]; }

Lexer& Lexer::operator++() {
    LA[pos] = advance();
    pos     = (pos + 1) % la;
    return *this;
}
