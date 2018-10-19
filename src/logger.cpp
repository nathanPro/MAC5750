#include "logger.h"

Logger::Logger(std::vector<AST::ErrorData>& _in) : errors(_in) {}

void Logger::push(std::string label, int line) {
    context.push_back(label);
    lines.push_back(line);
}

void Logger::pop() {
    context.pop_back();
    lines.pop_back();
}

void Logger::mismatch(Lexeme expected, Lexeme found, AST::Node id) {
    auto err   = std::make_unique<AST::ParsingError>();
    err->inner = AST::Mismatch{found, expected};
    err->ctx   = context;
    errors[id.get()].push_back(std::move(err));
}

void Logger::mismatch(std::string expected, std::string found,
                      AST::Node id) {
    auto err   = std::make_unique<AST::ParsingError>();
    err->inner = AST::WrongIdentifier{expected, found};
    err->ctx   = context;
    errors[id.get()].push_back(std::move(err));
};

void Logger::unexpected(Lexeme un, AST::Node id) {
    if (un == Lexeme::eof) return;
    auto err   = std::make_unique<AST::ParsingError>();
    err->inner = AST::Unexpected{un};
    err->ctx   = context;
    errors[id.get()].push_back(std::move(err));
}
