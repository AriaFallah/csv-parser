#include "../parser.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Workload {
  std::string name;
  std::string csv;
};

struct Result {
  std::string workload;
  std::string mode;
  std::size_t bytes;
  int iterations;
  double best_ms;
  double bytes_per_second;
  std::size_t checksum;
};

volatile std::size_t g_sink = 0;

auto repeat_char(char c, std::size_t count) -> std::string {
  return std::string(count, c);
}

auto make_plain_rows(std::size_t rows, std::size_t cols) -> std::string {
  std::string out;
  out.reserve(rows * cols * 8);

  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t col = 0; col < cols; ++col) {
      if (col != 0) {
        out += ',';
      }
      out += "field";
      out += static_cast<char>('0' + (col % 10));
    }
    out += '\n';
  }

  return out;
}

auto make_quoted_rows(std::size_t rows) -> std::string {
  std::string out;
  out.reserve(rows * 64);

  for (std::size_t row = 0; row < rows; ++row) {
    out += "\"alpha,beta\",\"said \"\"hello\"\"\",\"line ";
    out += static_cast<char>('0' + (row % 10));
    out += "\nnext\",tail\n";
  }

  return out;
}

auto make_wide_rows(std::size_t rows, std::size_t cols) -> std::string {
  std::string out;
  out.reserve(rows * cols * 3);

  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t col = 0; col < cols; ++col) {
      if (col != 0) {
        out += ',';
      }
      out += static_cast<char>('a' + (col % 26));
    }
    out += '\n';
  }

  return out;
}

auto make_huge_fields(std::size_t rows, std::size_t field_size) -> std::string {
  std::string out;
  const auto field = repeat_char('x', field_size);
  out.reserve(rows * (field_size + 4));

  for (std::size_t row = 0; row < rows; ++row) {
    out += field;
    out += ",z\n";
  }

  return out;
}

auto workloads() -> std::vector<Workload> {
  std::vector<Workload> out;
  out.push_back({"plain", make_plain_rows(50000, 12)});
  out.push_back({"quoted", make_quoted_rows(50000)});
  out.push_back({"wide", make_wide_rows(10000, 200)});
  out.push_back({"huge-fields", make_huge_fields(64, 256 * 1024)});
  return out;
}

auto parse_fields(const std::string &csv) -> std::size_t {
  std::istringstream input(csv);
  aria::csv::CsvParser parser(input);

  std::size_t checksum = 0;
  for (;;) {
    const auto field = parser.next_field();
    if (field.type == aria::csv::FieldType::CSV_END) {
      break;
    }
    checksum += static_cast<std::size_t>(field.type);
    checksum += field.data.size();
  }

  return checksum;
}

auto parse_rows(const std::string &csv) -> std::size_t {
  std::istringstream input(csv);
  aria::csv::CsvParser parser(input);

  std::size_t checksum = 0;
  for (const auto &row : parser) {
    checksum += row.size();
    for (const auto &field : row) {
      checksum += field.size();
    }
  }

  return checksum;
}

template <typename Fn>
auto time_best(const Workload &workload, const std::string &mode, int iterations,
               Fn fn) -> Result {
  double best_ms = 0.0;
  std::size_t checksum = 0;

  for (int i = 0; i < iterations; ++i) {
    const auto start = std::chrono::steady_clock::now();
    checksum += fn(workload.csv);
    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration<double, std::milli>(end - start).count();
    if (i == 0 || elapsed < best_ms) {
      best_ms = elapsed;
    }
  }

  g_sink = checksum;
  const double seconds = best_ms / 1000.0;
  return {workload.name,
          mode,
          workload.csv.size(),
          iterations,
          best_ms,
          static_cast<double>(workload.csv.size()) / seconds,
          checksum};
}

void print_header() {
  std::cout << "workload,mode,bytes,iterations,best_ms,mb_per_s,checksum\n";
}

void print_result(const Result &result) {
  std::cout << result.workload << ',' << result.mode << ',' << result.bytes
            << ',' << result.iterations << ',' << std::fixed
            << std::setprecision(3) << result.best_ms << ','
            << std::setprecision(2)
            << (result.bytes_per_second / (1024.0 * 1024.0)) << ','
            << result.checksum << '\n';
}

} // namespace

int main(int argc, char **argv) {
  const int iterations = argc > 1 ? std::atoi(argv[1]) : 5;
  if (iterations <= 0) {
    std::cerr << "usage: " << argv[0] << " [positive-iterations]\n";
    return 2;
  }

  const auto data = workloads();
  print_header();
  for (const auto &workload : data) {
    print_result(time_best(workload, "fields", iterations, parse_fields));
    print_result(time_best(workload, "rows", iterations, parse_rows));
  }
}
