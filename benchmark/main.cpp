#include <iostream>
#include "../parser.hpp"

using namespace aria::csv;

int main() {
  CsvReader reader("./ss10pusa.csv");
  while (reader.get_row()) {}
  std::cout << "done!\n";
}
