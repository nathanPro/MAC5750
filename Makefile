GTEST_DIR = googletest/googletest
TEST_DIR = test
BIN_DIR = bin
HDR_DIR = gch

CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0 -DDEBUG -I src
GTESTFLAGS = -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread

IR_OBJS = $(BIN_DIR)/IR.o $(BIN_DIR)/IRBuilder.o

test: $(TEST_DIR)/libgtest.a $(BIN_DIR)/IR.o $(BIN_DIR)/IRBuilder.o
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(IR_OBJS) $(TEST_DIR)/IRtest.cpp $(TEST_DIR)/libgtest.a -o $(TEST_DIR)/IRtest
	./test/IRtest
$(BIN_DIR)/IR.o: src/IR.h src/IR.cpp 
	$(CXX) $(CXXFLAGS) -c src/IR.cpp -o $(BIN_DIR)/IR.o
$(BIN_DIR)/IRBuilder.o: src/IRBuilder.h src/IRBuilder.cpp
	$(CXX) $(CXXFLAGS) -c src/IRBuilder.cpp -o $(BIN_DIR)/IRBuilder.o
$(TEST_DIR)/libgtest.a:
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o $(TEST_DIR)/gtest-all.o
	ar -rv $(TEST_DIR)/libgtest.a $(TEST_DIR)/gtest-all.o
$(BIN_DIR)/lexer.o:
	$(CXX) $(CXXFLAGS) -c src/lexer.cpp -o $(BIN_DIR)/lexer.o
bcc: $(BIN_DIR)/lexer.o src/error.h src/AST.h src/parser.h src/lexer.h src/main.cpp
	$(CXX) $(CXXFLAGS) $(BIN_DIR)/lexer.o src/main.cpp -o $(BIN_DIR)/bcc
all: test bcc

.PHONY: clean
clean:
	-rm $(TEST_DIR)/*.a
	-rm $(TEST_DIR)/*.o
	-rm $(TEST_DIR)/IRtest
	-rm $(BIN_DIR)/*
	-rm $(HDR_DIR)/*
