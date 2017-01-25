CXX = clang++
CXXFLAGS = -std=c++11

all: main.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf *.o*
