bcc: parser.o ../src/error.h ../src/AST.h ../src/parser.h \
    ../src/lexer.h ../src/main.cpp
	$(CXX) $(CXXFLAGS) lexer.o logger.o Builder.o parser.o ../src/main.cpp -o bcc
IR.o: ../src/IR.h ../src/IR.cpp
	$(CXX) $(CXXFLAGS) -c ../src/IR.cpp -o IR.o
IRBuilder.o: ../src/IRBuilder.h ../src/IRBuilder.cpp
	$(CXX) $(CXXFLAGS) -c ../src/IRBuilder.cpp \
	    -o IRBuilder.o
lexer.o: ../src/lexer.h ../src/lexer.cpp
	$(CXX) $(CXXFLAGS) -c ../src/lexer.cpp
logger.o: ../src/logger.h ../src/logger.cpp
	$(CXX) $(CXXFLAGS) -c ../src/logger.cpp
Builder.o: ../src/Builder.h ../src/Builder.cpp lexer.o logger.o
	$(CXX) $(CXXFLAGS) -c ../src/Builder.cpp lexer.o logger.o -o Builder.o
parser.o: ../src/parser.h ../src/parser.cpp lexer.o logger.o Builder.o
	$(CXX) $(CXXFLAGS) -c ../src/parser.cpp lexer.o logger.o Builder.o -o parser.o
