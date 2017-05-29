#include <fstream>
#include <iostream>
#include "../../parser.hpp"

using namespace aria::csv;

int main(int, char **argv) {
  int count = 0;
  std::ifstream f(argv[1]);
  CsvParser parser(f);

  for (auto& row : parser) {
    ++count;
  }

  std::cout << count << std::endl;
}
