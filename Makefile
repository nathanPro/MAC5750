GTEST_DIR = googletest/googletest
CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0 -DDEBUG
GTESTFLAGS = -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread
test: libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) IRtest.cpp libgtest.a -o IRtest
libgtest.a:
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
main: lexer.o error.h AST.h.gch parser.h.gch lexer.h.gch main.cpp
	$(CXX) $(CXXFLAGS) lexer.o main.cpp -o main
Builder.h.gch: Builder.h
	$(CXX) $(CXXFLAGS) Builder.h
AST.h.gch: AST.h
	$(CXX) $(CXXFLAGS) AST.h
parser.h.gch: parser.h Builder.h.gch
	$(CXX) $(CXXFLAGS) parser.h
lexer.h.gch: lexer.h
	$(CXX) $(CXXFLAGS) lexer.h
all: test main

.PHONY: clean
clean:
	rm *.gch
	rm *.o
	rm main
