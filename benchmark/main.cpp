#include <iostream>
#include "../parser.hpp"

using namespace aria::csv;

int main(int argc, char **argv) {
  if (argc < 2) return 1;

  CsvParser parser(argv[1]);
  while (true) {
    auto field = parser.next_field();
    if (field.type == FieldType::CSV_END) {
      break;
    }
  }
}
