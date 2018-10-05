GTEST_DIR = googletest/googletest
GTESTFLAGS = -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread

.PHONY: test catamorphism IRtest translation
test: IRtest catamorphism

catamorphism: catamorphism.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(OBJS) \
	catamorphism.cpp libgtest.a -o catamorphism
	./catamorphism

IRtest: IRtest.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(OBJS) \
	IRtest.cpp libgtest.a -o IRtest
	./IRtest
translation: translation.cpp libgtest.a
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) $(OBJS) \
	translation.cpp libgtest.a -o translationtest
	./translationtest

libgtest.a:
	$(CXX) $(CXXFLAGS) $(GTESTFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o gtest-all.o
	ar -rv libgtest.a gtest-all.o
