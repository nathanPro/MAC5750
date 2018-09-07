CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0

parser: lexer.o AST.o
