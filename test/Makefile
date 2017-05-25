CXX = clang++
CXXFLAGS = -std=c++11

all: parser_test.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf *.o*
