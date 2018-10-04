GTEST_DIR = googletest/googletest
GTESTFLAGS = -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread

test: IRtest.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) \
	../$(BIN_DIR)/IR.o ../$(BIN_DIR)/IRBuilder.o \
	IRtest.cpp libgtest.a -o IRtest
	./IRtest
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) \
	translation.cpp libgtest.a -o translationtest
	./translationtest

libgtest.a:
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o gtest-all.o
	ar -rv libgtest.a gtest-all.o
