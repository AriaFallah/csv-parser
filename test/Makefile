CXX = clang++
CXXFLAGS = -std=c++11

all: infer_test.o parser_test.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf *.o*
