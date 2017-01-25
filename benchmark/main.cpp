#include <iostream>
#include "../parser.hpp"

using namespace aria::csv;

int main(int argc, char** argv) {
  if (argc == 0) return 1;
  CsvReader reader(argv[1]);
  for (;;) {
    auto row = reader.get_row();
    if (row.empty()) break;
  }
  std::cout << "done!\n";
}
