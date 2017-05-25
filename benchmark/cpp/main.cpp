#include <fstream>
#include <iostream>
#include "../../parser.hpp"

using namespace aria::csv;

int main(int, char **argv) {
  int count = 0;
  std::ifstream f(argv[1]);
  CsvParser parser(f);

  while (true) {
    auto field = parser.next_field();
    if (field.type == FieldType::ROW_END) {
      ++count;
    }
    if (field.type == FieldType::CSV_END) {
      break;
    }
  }

  std::cout << count << std::endl;
}
