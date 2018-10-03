GTEST_DIR = googletest/googletest
TEST_DIR = test
BIN_DIR = bin

CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0 -DDEBUG -I src
GTESTFLAGS = -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread

IR_OBJS = $(BIN_DIR)/IR.o $(BIN_DIR)/IRBuilder.o

test: libgtest IR
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(IR_OBJS) $(TEST_DIR)/IRtest.cpp $(TEST_DIR)/libgtest.a -o $(TEST_DIR)/IRtest
	./test/IRtest
IR: src/IR.h src/IR.cpp src/IRBuilder.h src/IRBuilder.cpp
	$(CXX) $(CXXFLAGS) -c src/IR.cpp -o $(BIN_DIR)/IR.o
	$(CXX) $(CXXFLAGS) -c src/IRBuilder.cpp -o $(BIN_DIR)/IRBuilder.o
libgtest:
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o $(TEST_DIR)/gtest-all.o
	ar -rv $(TEST_DIR)/libgtest.a gtest-all.o

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
