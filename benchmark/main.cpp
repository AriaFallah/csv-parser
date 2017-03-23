#include <iostream>
#include "../parser.hpp"

using namespace aria::csv;

int main(int argc, char **argv) {
  if (argc < 2) return 1;

  CsvReader reader(argv[1]);
  while (reader.get_row()) {}
  std::cout << "done!\n";
}
