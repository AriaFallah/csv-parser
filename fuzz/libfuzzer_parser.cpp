#include "../parser.hpp"

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string input(reinterpret_cast<const char *>(data), size);
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
