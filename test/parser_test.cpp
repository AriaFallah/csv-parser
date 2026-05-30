#include "../parser.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>

using namespace aria::csv;

auto read_all(CsvParser &p) -> CSV {
  CSV csv;
  for (const auto &row : p) {
    csv.push_back(row);
  }
  return csv;
}

auto parse_string(const std::string &input) -> CSV {
  std::istringstream stream(input);
  CsvParser parser(stream);
  return read_all(parser);
}

TEST(CsvParserTest, CommaInQuotes) {
  std::ifstream f(TEST_DATA_DIR "/comma_in_quotes.csv");
  CsvParser parser(f);
  CSV expected = {{"first", "last", "address", "city", "zip"},
                  {"John", "Doe", "120 any st.", "Anytown, WW", "08123"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, Empty) {
  std::ifstream f(TEST_DATA_DIR "/empty.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"}, {"1", "", ""}, {"2", "3", "4"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, EmptyUnquoted) {
  std::ifstream f(TEST_DATA_DIR "/emptyUnquoted.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"}, {"1", "", ""}, {"2", "3", "4"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, EmptyCrlf) {
  std::ifstream f(TEST_DATA_DIR "/empty_crlf.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"}, {"1", "", ""}, {"2", "3", "4"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, EscapedQuotes) {
  std::ifstream f(TEST_DATA_DIR "/escaped_quotes.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b"}, {"1", R"(ha "ha" ha)"}, {"3", "4"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, Json) {
  std::ifstream f(TEST_DATA_DIR "/json.csv");
  CsvParser parser(f);
  CSV expected = {{"key", "val"},
                  {"1", R"({"type": "Point", "coordinates": [102.0, 0.5]})"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, Newlines) {
  std::ifstream f(TEST_DATA_DIR "/newlines.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"},
                  {"1", "2", "3"},
                  {"Once upon \na time", "5", "6"},
                  {"7", "8", "9"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, NewlinesCrlf) {
  std::ifstream f(TEST_DATA_DIR "/newlines_crlf.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"},
                  {"1", "2", "3"},
                  {"Once upon \r\na time", "5", "6"},
                  {"7", "8", "9"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, QuotesAndNewlines) {
  std::ifstream f(TEST_DATA_DIR "/quotes_and_newlines.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b"}, {"1", "ha \n\"ha\" \nha"}, {"3", "4"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, Simple) {
  std::ifstream f(TEST_DATA_DIR "/simple.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"}, {"1", "2", "3"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, SimpleCrlf) {
  std::ifstream f(TEST_DATA_DIR "/simple_crlf.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"}, {"1", "2", "3"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, Utf8) {
  std::ifstream f(TEST_DATA_DIR "/utf8.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"}, {"1", "2", "3"}, {"4", "5", "ʤ"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, DifferentDelimiter) {
  std::ifstream f(TEST_DATA_DIR "/delimiter.csv");
  CsvParser parser = CsvParser(f).delimiter(';');
  CSV expected = {{"a", "b", "c"}, {"1", "2", "3"}, {"4", "5", ","}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, DifferentTerminator) {
  std::ifstream f(TEST_DATA_DIR "/terminator.csv");
  CsvParser parser = CsvParser(f).terminator(';');
  CSV expected = {{"a", "b", "c"}, {"1", "2", "3"}, {"4", "5", "6\n"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, DifferentQuote) {
  std::ifstream f(TEST_DATA_DIR "/quote.csv");
  CsvParser parser = CsvParser(f).quote('\'');
  CSV expected = {{"1, 2, 3", "4, 5, 6", "\n7\n8\n9"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, BomSimple) {
  std::ifstream f(TEST_DATA_DIR "/bom_simple.csv");
  CsvParser parser(f);
  CSV expected = {{"a", "b", "c"}, {"1", "2", "3"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, BomEmpty) {
  std::ifstream f(TEST_DATA_DIR "/bom_empty.csv");
  CsvParser parser(f);
  CSV expected = {};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, EmptyFile) {
  std::ifstream f(TEST_DATA_DIR "/empty_file.csv");
  CsvParser parser(f);
  CSV expected = {};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, TrailingEmptyFieldAtEof) {
  CSV expected = {{"a", "b", ""}};
  EXPECT_EQ(parse_string("a,b,"), expected);
}

TEST(CsvParserTest, EmptyQuotedFieldAtEof) {
  CSV expected = {{""}};
  EXPECT_EQ(parse_string("\"\""), expected);
}

TEST(CsvParserTest, UnclosedEmptyQuotedFieldAtEof) {
  CSV expected = {{""}};
  EXPECT_EQ(parse_string("\""), expected);
}

TEST(CsvParserTest, EmptyFieldsAtEof) {
  CSV expected = {{"", ""}};
  EXPECT_EQ(parse_string(","), expected);
}

TEST(CsvParserTest, EmptyLineStartsWithRowEnd) {
  std::istringstream stream("\n\n");
  CsvParser parser(stream);

  const auto first = parser.next_field();
  const auto second = parser.next_field();

  EXPECT_EQ(first.type, FieldType::ROW_END);
  EXPECT_EQ(second.type, FieldType::ROW_END);
}

TEST(CsvParserTest, FinalLineDoesNotNeedTerminator) {
  CSV expected = {{"a", "b"}, {"1", "2"}};
  EXPECT_EQ(parse_string("a,b\n1,2"), expected);
}

TEST(CsvParserTest, PositionTracksParsedTokensNotBufferedBytes) {
  std::istringstream stream("abc,def");
  CsvParser parser(stream);

  const auto field = parser.next_field();

  EXPECT_EQ(field.type, FieldType::DATA);
  EXPECT_EQ(field.data, "abc");
  EXPECT_EQ(parser.position(), 4);
}

TEST(CsvParserTest, OwnsInputStreamWhenConstructedWithUniquePtr) {
  std::unique_ptr<std::istream> stream(new std::istringstream("a,b\n1,2"));
  CsvParser parser(std::move(stream));
  CSV expected = {{"a", "b"}, {"1", "2"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, RejectsNullOwnedInputStream) {
  std::unique_ptr<std::istream> stream;
  EXPECT_THROW(CsvParser parser(std::move(stream)), std::invalid_argument);
}

TEST(CsvParserTest, FromFileOwnsInputStream) {
  CsvParser parser = CsvParser::from_file(TEST_DATA_DIR "/simple.csv");
  CSV expected = {{"a", "b", "c"}, {"1", "2", "3"}};
  EXPECT_EQ(read_all(parser), expected);
}

TEST(CsvParserTest, FromFileRejectsMissingFile) {
  EXPECT_THROW(CsvParser::from_file(TEST_DATA_DIR "/does_not_exist.csv"),
               std::runtime_error);
}
