#define CATCH_PLATFORM_MAC
#define CATCH_CONFIG_MAIN

#include <fstream>
#include "./catch.hpp"
#include "../parser.hpp"

using namespace aria::csv;

auto read_all(CsvParser& p) -> CSV {
  CSV csv;
  for (const auto& row : p) {
    csv.push_back(row);
  }
  return csv;
}

TEST_CASE("comma in quotes") {
  std::ifstream f("./data/comma_in_quotes.csv");
  CsvParser parser(f);
  CSV expected = {
    { "first", "last", "address", "city", "zip" },
    { "John", "Doe", "120 any st.", "Anytown, WW", "08123" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("empty") {
  std::ifstream f("./data/empty.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "", "" },
    { "2", "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("emptyUnquoted") {
  std::ifstream f("./data/emptyUnquoted.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "", "" },
    { "2", "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("empty crlf") {
  std::ifstream f("./data/empty_crlf.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "", "" },
    { "2", "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("escaped quotes") {
  std::ifstream f("./data/escaped_quotes.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b" },
    { "1", R"(ha "ha" ha)" },
    { "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("json") {
  std::ifstream f("./data/json.csv");
  CsvParser parser(f);
  CSV expected = {
    { "key","val" },
    { "1", R"({"type": "Point", "coordinates": [102.0, 0.5]})" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("newlines") {
  std::ifstream f("./data/newlines.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "Once upon \na time", "5", "6" },
    { "7", "8", "9" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("newlines crlf") {
  std::ifstream f("./data/newlines_crlf.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "Once upon \r\na time", "5", "6" },
    { "7", "8", "9" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("quotes and newlines") {
  std::ifstream f("./data/quotes_and_newlines.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b" },
    { "1", "ha \n\"ha\" \nha" },
    { "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("simple") {
  std::ifstream f("./data/simple.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("simple crlf") {
  std::ifstream f("./data/simple_crlf.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("utf8") {
  std::ifstream f("./data/utf8.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "4", "5", "ʤ" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("different delimiter") {
  std::ifstream f("./data/delimiter.csv");
  CsvParser parser = CsvParser(f)
    .delimiter(';');
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "4", "5", "," }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("different terminator") {
  std::ifstream f("./data/terminator.csv");
  CsvParser parser = CsvParser(f)
    .terminator(';');
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "4", "5", "6\n" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("different quote") {
  std::ifstream f("./data/quote.csv");
  CsvParser parser = CsvParser(f)
    .quote('\'');
  CSV expected = {
    { "1, 2, 3", "4, 5, 6", "\n7\n8\n9" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("BOM simple") {
  std::ifstream f("./data/bom_simple.csv");
  CsvParser parser(f);
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("BOM empty") {
  std::ifstream f("./data/bom_empty.csv");
  CsvParser parser(f);
  CSV expected = {};
  CHECK(read_all(parser) == expected);
}

TEST_CASE("empty_file") {
    std::ifstream f("./data/empty_file.csv");
    CsvParser parser(f);
    CSV expected = {};
    CHECK(read_all(parser) == expected);
}
