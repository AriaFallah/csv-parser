#include "../parser.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

volatile std::size_t g_sink = 0;

auto read_file(const char *path) -> std::string {
  std::ifstream file(path, std::ios::binary);
  if (!file.good()) {
    throw std::runtime_error("failed to open input file");
  }

  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

auto parse_fields(const std::string &input) -> std::size_t {
  std::istringstream stream(input);
  aria::csv::CsvParser parser(stream);

  std::size_t fields = 0;
  for (;;) {
    const auto field = parser.next_field();
    if (field.type == aria::csv::FieldType::CSV_END) {
      break;
    }
    if (field.type == aria::csv::FieldType::DATA) {
      fields += field.data.size() + 1;
    }
  }

  return fields;
}

} // namespace

int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    std::cerr << "usage: " << argv[0] << " <csv-file> [iterations]\n";
    return 2;
  }

  const int iterations = argc == 3 ? std::atoi(argv[2]) : 1000;
  const auto input = read_file(argv[1]);

  std::size_t parsed = 0;
  for (int i = 0; i < iterations; ++i) {
    parsed += parse_fields(input);
  }

  g_sink = parsed;
  std::cout << "bytes=" << input.size() << " iterations=" << iterations
            << " parsed=" << parsed << '\n';
}
