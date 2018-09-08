CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0 -DDEBUG

main: lexer.o AST.h parser.h lexer.h

.PHONY: clean
clean:
	rm *.o
	rm main
