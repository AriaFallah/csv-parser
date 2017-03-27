#define CATCH_CONFIG_MAIN

#include "./catch.hpp"
#include "../parser.hpp"

using namespace aria::csv;

CSV read_all(CsvParser& p) {
  CSV csv;
  for (auto& row : p) {
    std::vector<std::string> r;
    for (auto& field : row) {
      r.push_back(field);
    }
    csv.push_back(r);
  }
  return csv;
}

TEST_CASE("comma in quotes") {
  CsvParser parser("./data/comma_in_quotes.csv");
  CSV expected = {
    { "first", "last", "address", "city", "zip" },
    { "John", "Doe", "120 any st.", "Anytown, WW", "08123" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("empty") {
  CsvParser parser("./data/empty.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "", "" },
    { "2", "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("empty crlf") {
  CsvParser parser("./data/empty_crlf.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "", "" },
    { "2", "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("escaped quotes") {
  CsvParser parser("./data/escaped_quotes.csv");
  CSV expected = {
    { "a", "b" },
    { "1", R"(ha "ha" ha)" },
    { "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("json") {
  CsvParser parser("./data/json.csv");
  CSV expected = {
    { "key","val" },
    { "1", R"({"type": "Point", "coordinates": [102.0, 0.5]})" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("newlines") {
  CsvParser parser("./data/newlines.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "Once upon \na time", "5", "6" },
    { "7", "8", "9" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("newlines crlf") {
  CsvParser parser("./data/newlines_crlf.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "Once upon \r\na time", "5", "6" },
    { "7", "8", "9" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("quotes and newlines") {
  CsvParser parser("./data/quotes_and_newlines.csv");
  CSV expected = {
    { "a", "b" },
    { "1", "ha \n\"ha\" \nha" },
    { "3", "4" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("simple") {
  CsvParser parser("./data/simple.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("simple crlf") {
  CsvParser parser("./data/simple_crlf.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" }
  };
  CHECK(read_all(parser) == expected);
}

TEST_CASE("utf8") {
  CsvParser parser("./data/utf8.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "4", "5", "ʤ" }
  };
  CHECK(read_all(parser) == expected);
}
