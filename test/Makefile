GTEST_DIR = googletest/googletest
GTESTFLAGS = -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread

.PHONY: test
test: helper translation IR catamorphism

helper: helper.cpp ../src/helper.cpp ../src/helper.h libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(OBJS) \
	    helper.cpp libgtest.a -o helper
	./helper
catamorphism: catamorphism.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(OBJS) \
	catamorphism.cpp libgtest.a -o catamorphism
	./catamorphism
IR: IR.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(OBJS) \
	IR.cpp libgtest.a -o IR
	./IR
translation: translation.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(OBJS) \
	translation.cpp libgtest.a -o translation
	./translation

libgtest.a:
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o gtest-all.o
	ar -rv libgtest.a gtest-all.o
