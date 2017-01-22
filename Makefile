CXX = clang++
CXXFLAGS = -std=c++14

main.out: main.cpp parser.hpp
	$(CXX) $(CXXFLAGS) main.cpp -o main.out

lint:
	clang-tidy parser.hpp -header-filter=.* -checks=cppcoreguidelines-*,clang-analyzer-*,modernize-*,performance-*,misc-*,cert-*,llvm-* -- -std=c++14
