# CSV Parser

Fast, header-only C++11 CSV parser.

## Usage

#### Configuration

You initialize the parser by passing it any input stream of characters. For example,
you can read from a file

```cpp
std::ifstream f("some_file.csv");
CsvParser parser(f);
```

or you can read from `stdin`

```cpp
CsvParser parser(std::cin);
```

Moreover, you can configure the parser by chaining configuration methods like

```cpp
CsvParser parser = CsvParser(std::cin)
  .delimiter(';')    // delimited by ; instead of ,
  .quote('\'')       // quoted fields use ' instead of "
  .terminator('\0'); // terminated by \0 instead of by \r\n, \n, or \r
```

#### Parsing

You can read from the CSV using a range based for loop. Each row of the CSV
is represented as a `std::vector<std::string>`.

```cpp
#include <iostream>
#include "../parser.hpp"

using namespace aria::csv;

int main() {
  std::ifstream f("some_file.csv");
  CsvParser parser(f);

  for (auto& row : parser) {
    for (auto& field : row) {
      std::cout << field << " | ";
    }
    std::cout << std::endl;
  }
}
```
