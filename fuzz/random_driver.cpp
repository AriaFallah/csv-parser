#include "../parser.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void parse_one(const std::string &input) {
  std::istringstream field_stream(input);
  try {
    aria::csv::CsvParser parser(field_stream);
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
}

auto read_file(const char *path) -> std::string {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    throw std::runtime_error(std::string("failed to open seed: ") + path);
  }

  std::ostringstream out;
  out << file.rdbuf();
  return out.str();
}

auto next_random(uint64_t &state) -> uint64_t {
  state ^= state << 13;
  state ^= state >> 7;
  state ^= state << 17;
  return state;
}

auto random_byte(uint64_t &state) -> char {
  static const char interesting[] = {
      '\0', '\n', '\r', ',', '"', '\\', 'a', '0', static_cast<char>(0xEF),
      static_cast<char>(0xBB), static_cast<char>(0xBF), static_cast<char>(0xFF)};
  const uint64_t value = next_random(state);
  if ((value & 3U) == 0U) {
    return interesting[value % (sizeof(interesting) / sizeof(interesting[0]))];
  }
  return static_cast<char>((value >> 8) & 0xFFU);
}

void mutate(std::string &input, uint64_t &state) {
  const uint64_t action = next_random(state) % 6U;
  if (input.empty() || action == 0U) {
    const size_t pos = input.empty() ? 0U : next_random(state) % (input.size() + 1U);
    input.insert(input.begin() + static_cast<std::ptrdiff_t>(pos),
                 random_byte(state));
    return;
  }

  const size_t pos = next_random(state) % input.size();
  switch (action) {
  case 1:
    input[pos] = random_byte(state);
    break;
  case 2:
    input.erase(input.begin() + static_cast<std::ptrdiff_t>(pos));
    break;
  case 3: {
    const size_t len = 1U + (next_random(state) % 64U);
    input.insert(pos, len, random_byte(state));
    break;
  }
  case 4:
    input.resize(next_random(state) % 4096U);
    break;
  default:
    input += input.substr(pos, input.size() - pos);
    if (input.size() > 8192U) {
      input.resize(8192U);
    }
    break;
  }
}

} // namespace

int main(int argc, char **argv) {
  std::vector<std::string> corpus;
  for (int i = 1; i < argc; ++i) {
    corpus.push_back(read_file(argv[i]));
  }
  if (corpus.empty()) {
    corpus.push_back("");
    corpus.push_back("a,b,c\n1,2,3\n");
    corpus.push_back("\"a\",\"b\nb\",\"c\"\r\n");
  }

  const char *runs_env = std::getenv("ARIA_CSV_FUZZ_RUNS");
  const size_t runs = runs_env == nullptr ? 100000U : std::strtoull(runs_env, nullptr, 10);
  uint64_t state = 0x9e3779b97f4a7c15ULL;

  for (size_t i = 0; i < runs; ++i) {
    std::string input = corpus[next_random(state) % corpus.size()];
    const size_t mutations = 1U + (next_random(state) % 32U);
    for (size_t j = 0; j < mutations; ++j) {
      mutate(input, state);
    }
    parse_one(input);
  }

  std::cerr << "fuzzed " << runs << " inputs\n";
  return 0;
}
