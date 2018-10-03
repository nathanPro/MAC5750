export TEST_DIR = test
export BIN_DIR = bin

export CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0 -DDEBUG -I ../src

test: $(BIN_DIR)/IR.o $(BIN_DIR)/IRBuilder.o
	$(MAKE) -C test
bcc: $(BIN_DIR)/lexer.o src/error.h src/AST.h src/parser.h \
    src/lexer.h src/main.cpp
	$(MAKE) -C bin
all: test bcc

$(BIN_DIR)/IR.o: src/IR.cpp src/IR.h
	$(MAKE) IR.o -C bin
$(BIN_DIR)/IRBuilder.o: src/IRBuilder.cpp src/IRBuilder.cpp
	$(MAKE) IRBuilder.o -C bin
$(BIN_DIR)/lexer.o: src/lexer.cpp src/lexer.h
	$(MAKE) lexer.o -C bin

.PHONY: clean, test
clean:
	-rm $(TEST_DIR)/*.a
	-rm $(TEST_DIR)/*.o
	-rm $(TEST_DIR)/IRtest
	-rm $(BIN_DIR)/*.o
	-rm $(BIN_DIR)/bcc
