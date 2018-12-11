export TEST_DIR = test
export BIN_DIR = bin

export CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0 -DDEBUG -I ../src

OBJSDEP = $(BIN_DIR)/IR.o \
          $(BIN_DIR)/IRBuilder.o \
	  $(BIN_DIR)/logger.o \
          $(BIN_DIR)/lexer.o \
          $(BIN_DIR)/parser.o \
	  $(BIN_DIR)/Builder.o

export OBJS = ../$(BIN_DIR)/IR.o \
              ../$(BIN_DIR)/IRBuilder.o \
              ../$(BIN_DIR)/logger.o \
              ../$(BIN_DIR)/lexer.o \
              ../$(BIN_DIR)/parser.o \
	      ../$(BIN_DIR)/Builder.o

test: $(OBJSDEP)
	$(MAKE) -C test

helper_test: $(OBJSDEP)
	$(MAKE) -C test helper_test

bcc: $(OBJSDEP) src/error.h src/AST.h src/parser.h \
    src/lexer.h src/main.cpp
	$(MAKE) -C bin
all: test bcc

$(BIN_DIR)/IR.o: src/IR.cpp src/IR.h
	$(MAKE) IR.o -C bin
$(BIN_DIR)/IRBuilder.o: src/IRBuilder.cpp src/IRBuilder.cpp
	$(MAKE) IRBuilder.o -C bin
$(BIN_DIR)/lexer.o: src/lexer.cpp src/lexer.h
	$(MAKE) lexer.o -C bin
$(BIN_DIR)/logger.o: src/logger.cpp src/logger.h
	$(MAKE) logger.o -C bin
$(BIN_DIR)/Builder.o: src/Builder.cpp src/Builder.h $(BIN_DIR)/logger.o
	$(MAKE) Builder.o -C bin
$(BIN_DIR)/parser.o: src/parser.cpp src/parser.h $(BIN_DIR)/lexer.o $(BIN_DIR)/logger.o $(BIN_DIR)/Builder.o
	$(MAKE) parser.o -C bin

.PHONY: clean, test
clean:
	-rm $(TEST_DIR)/*.a
	-rm $(TEST_DIR)/*.o
	-rm $(TEST_DIR)/IRtest
	-rm $(BIN_DIR)/*.o
	-rm $(BIN_DIR)/bcc
