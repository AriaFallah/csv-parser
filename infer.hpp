#ifndef ARIA_INFER_CSV_H
#define ARIA_INFER_CSV_H

#include <array>
#include <regex>
#include <string>
#include <thread>
#include <vector>

namespace aria {
  namespace csv {
    using CSV = std::vector<std::vector<std::string>>;
    enum class Type : int { INT, FLOAT, DATE, STRING, NONE };
    constexpr std::array<int, 5> rank{{ 1, 2, 4, 3, 0 }};

    class Infer {
    public:
      static auto infer_csv(const CSV& csv) -> std::vector<Type> {
        if (csv.empty()) return {};

        const int numCols = csv[0].size();
        std::vector<std::thread> threads;
        std::vector<Type> col_types(numCols);

        for (int col = 0; col < numCols; ++col) {
          threads.push_back(std::thread{infer_col, &csv, col, &col_types[col]});
        }

        for (auto& thread : threads) {
          thread.join();
        }

        return col_types;
      }
    private:
      static const std::regex is_int;
      static const std::regex is_float;
      static const std::regex is_date;

      static auto infer_col(const CSV *csv, const int col, Type *t) -> void {
        Type col_type = Type::NONE;

        for (const auto& row : *csv) {
          Type cell_type = get_type(row[col]);
          if (rank[static_cast<int>(cell_type)] > rank[static_cast<int>(col_type)]) {
            col_type = cell_type;
          }
        }

        *t = col_type;
      }

      static auto get_type(const std::string& s) -> Type {
        if (s.empty()) {
          return Type::NONE;
        } else if (std::regex_match(s, is_int)) {
          return Type::INT;
        } else if (std::regex_match(s, is_float)) {
          return Type::FLOAT;
        } else if (std::regex_match(s, is_date)) {
          return Type::DATE;
        } else {
          return Type::STRING;
        }
      }
    };

    const std::regex Infer::is_int   {R"(^\d+$)"};
    const std::regex Infer::is_float {R"(^\d*\.\d+$)"};
    const std::regex Infer::is_date  {R"(^\d{4}-\d{2}-\d{2}$)"};
  }
}

#endif
