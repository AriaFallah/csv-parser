#include "../parser.hpp"

#include <rapidcheck.h>

#include <sstream>
#include <string>

using namespace aria::csv;

namespace {

auto read_all(CsvParser &p) -> CSV {
  CSV csv;
  for (const auto &row : p) {
    csv.push_back(row);
  }
  return csv;
}

auto needs_quotes(const std::string &field) -> bool {
  return field.empty() || field.find_first_of(",\"\r\n") != std::string::npos;
}

auto write_csv(const CSV &rows) -> std::string {
  std::string out;

  for (CSV::size_type row = 0; row < rows.size(); ++row) {
    for (std::vector<std::string>::size_type col = 0; col < rows[row].size();
         ++col) {
      if (col != 0) {
        out += ',';
      }

      const auto &field = rows[row][col];
      if (!needs_quotes(field)) {
        out += field;
        continue;
      }

      out += '"';
      for (const auto c : field) {
        if (c == '"') {
          out += "\"\"";
        } else {
          out += c;
        }
      }
      out += '"';
    }

    if (row + 1 != rows.size()) {
      out += '\n';
    }
  }

  return out;
}

auto has_no_zero_field_rows(const CSV &rows) -> bool {
  for (const auto &row : rows) {
    if (row.empty()) {
      return false;
    }
  }
  return true;
}

} // namespace

int main() {
  rc::check("RFC-style serialized tables round-trip through CsvParser",
            [](const CSV &rows) {
              RC_PRE(has_no_zero_field_rows(rows));

              const auto text = write_csv(rows);
              std::istringstream input(text);
              CsvParser parser(input);

              RC_ASSERT(read_all(parser) == rows);
            });

  return 0;
}
