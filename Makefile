CXXFLAGS = --std=c++17 -Wall -Wextra -g -ggdb
CXX=clang++

AST: AST.h AST.cpp
	$(CXX) $(CXXFLAGS) AST.cpp -o AST
