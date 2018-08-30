CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb -O0

AST: AST.h AST.cpp
	$(CXX) $(CXXFLAGS) AST.cpp -o AST
