#include "../parser.hpp"

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

int main() {
  std::string input((std::istreambuf_iterator<char>(std::cin)),
                    std::istreambuf_iterator<char>());
  std::istringstream stream(input);

  try {
    aria::csv::CsvParser parser(stream);
    for (;;) {
      const auto field = parser.next_field();
      if (field.type == aria::csv::FieldType::CSV_END) {
        break;
      }
    }
  } catch (...) {
  }

  std::istringstream row_stream(input);
  try {
    aria::csv::CsvParser parser(row_stream);
    for (const auto &row : parser) {
      (void)row;
    }
  } catch (...) {
  }

  return 0;
}
