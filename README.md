# CSV Parser

Fast, header-only, C++11 CSV parser.

## Usage

#### Configuration

You initialize the parser by passing it any input stream of characters. For
example, you can read from a file

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

You can read from the CSV using a range based for loop. Each row of the CSV is
represented as a `std::vector<std::string>`.

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

Behind the scenes, when using the range based for, the parser only ever
allocates as much memory as needed to represent a single row of your CSV. If
that's too much, you can step down to a lower level, where you read from the CSV
a field at a time, which only allocates the amount of memory needed for a single
field.

```cpp
#include <iostream>
#include "./parser.hpp"

using namespace aria::csv;

int main() {
  CsvParser parser(std::cin);

  for (;;) {
    auto field = parser.next_field();
    switch (field.type) {
      case FieldType::DATA:
        std::cout << *field.data << " | ";
        break;
      case FieldType::ROW_END:
        std::cout << std::endl;
        break;
      case FieldType::CSV_END:
        std::cout << std::endl;
        return 0;
    }
  }
}
```

It is possible to inspect the current cursor position using `parser.position()`.
This will return the position of the last parsed token. This is useful when
reporting things like progress through a file. You can use
`file.seekg(0, std::ios::end);` to get a file size.
