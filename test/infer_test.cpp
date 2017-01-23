#define CATCH_CONFIG_MAIN

#include "./catch.hpp"
#include "../infer.hpp"

using namespace aria::csv;

TEST_CASE("it works") {
  CSV csv = {
    { "a", "1", "c", "1", "" },
    { "1", "2.0", "2015-10-10", "1", "" }
  };
  std::vector<Type> expected = {
    Type::STRING,
    Type::FLOAT,
    Type::DATE,
    Type::INT,
    Type::NONE
  };
  CHECK(Infer::infer_csv(csv) == expected);
}
