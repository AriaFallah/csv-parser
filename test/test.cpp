#define CATCH_CONFIG_MAIN

#include "./catch.hpp"
#include "../parser.hpp"

using CSV = std::vector<std::vector<std::string>>;

TEST_CASE("comma in quotes") {
  CsvReader reader("./data/comma_in_quotes.csv");
  CSV expected = {
    { "first", "last", "address", "city", "zip" },
    { "John", "Doe", "120 any st.", "Anytown, WW", "08123" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("empty") {
  CsvReader reader("./data/empty.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "", "" },
    { "2", "3", "4" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("empty clrf") {
  CsvReader reader("./data/empty_crlf.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "", "" },
    { "2", "3", "4" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("escaped quotes") {
  CsvReader reader("./data/escaped_quotes.csv");
  CSV expected = {
    { "a", "b" },
    { "1", R"(ha "ha" ha)" },
    { "3", "4" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("json") {
  CsvReader reader("./data/json.csv");
  CSV expected = {
    { "key","val" },
    { "1", R"({"type": "Point", "coordinates": [102.0, 0.5]})" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("newlines") {
  CsvReader reader("./data/newlines.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "Once upon \na time", "5", "6" },
    { "7", "8", "9" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("newlines clrf") {
  CsvReader reader("./data/newlines_crlf.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "Once upon \r\na time", "5", "6" },
    { "7", "8", "9" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("quotes and newlines") {
  CsvReader reader("./data/quotes_and_newlines.csv");
  CSV expected = {
    { "a", "b" },
    { "1", "ha \n\"ha\" \nha" },
    { "3", "4" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("simple") {
  CsvReader reader("./data/simple.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("simple clrf") {
  CsvReader reader("./data/simple_crlf.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" }
  };
  CHECK(reader.read_all() == expected);
}

TEST_CASE("utf8") {
  CsvReader reader("./data/utf8.csv");
  CSV expected = {
    { "a", "b", "c" },
    { "1", "2", "3" },
    { "4", "5", "ʤ" }
  };
  CHECK(reader.read_all() == expected);
}
