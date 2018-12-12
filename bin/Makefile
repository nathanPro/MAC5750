bcc:
	$(CXX) $(CXXFLAGS) lexer.o logger.o Builder.o parser.o ../src/main.cpp -o bcc
IR.o:
	$(CXX) $(CXXFLAGS) -c ../src/IR.cpp -o IR.o
IRBuilder.o:
	$(CXX) $(CXXFLAGS) -c ../src/IRBuilder.cpp -o IRBuilder.o
lexer.o:
	$(CXX) $(CXXFLAGS) -c ../src/lexer.cpp
logger.o:
	$(CXX) $(CXXFLAGS) -c ../src/logger.cpp
Builder.o: 
	$(CXX) $(CXXFLAGS) -c ../src/Builder.cpp -o Builder.o
parser.o: 
	$(CXX) $(CXXFLAGS) -c ../src/parser.cpp -o parser.o
class_graph.o: ../src/class_graph.h ../src/class_graph.cpp
	$(CXX) $(CXXFLAGS) -c ../src/class_graph.cpp -o class_graph.o
helper.o: ../src/helper.h ../src/helper.cpp
	$(CXX) $(CXXFLAGS) -c ../src/helper.cpp -o helper.o
translate.o: ../src/translate.h ../src/translate.cpp
	$(CXX) $(CXXFLAGS) -c ../src/translate.cpp -o translate.o
