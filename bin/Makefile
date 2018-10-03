bcc: lexer.o ../src/error.h ../src/AST.h ../src/parser.h \
    ../src/lexer.h ../src/main.cpp
	$(CXX) $(CXXFLAGS) lexer.o ../src/main.cpp -o bcc
lexer.o: ../src/lexer.h ../src/lexer.cpp
	$(CXX) $(CXXFLAGS) -c ../src/lexer.cpp
IR.o: ../src/IR.h ../src/IR.cpp
	$(CXX) $(CXXFLAGS) -c ../src/IR.cpp -o IR.o
IRBuilder.o: ../src/IRBuilder.h ../src/IRBuilder.cpp
	$(CXX) $(CXXFLAGS) -c ../src/IRBuilder.cpp \
	    -o IRBuilder.o
