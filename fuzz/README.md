# Fuzzing

This directory contains production fuzz targets for parser robustness. The
library remains dependency-free; fuzzing tools are build-time only.

## Harness Shape

```text
+----------------------+     +----------------------+     +----------------------+
| fuzzer input bytes   | --> | std::istringstream   | --> | CsvParser            |
+----------------------+     +----------------------+     +----------------------+
                                                            | next_field() path
                                                            | row iterator path
                                                            v
                                                   +----------------------+
                                                   | ASan/UBSan/crashes   |
                                                   +----------------------+
```

The harnesses catch parser exceptions because malformed CSV is allowed. Fuzzing
is looking for memory errors, undefined behavior, hangs, and crashes.

## libFuzzer

Use Clang with libFuzzer and sanitizers:

```sh
clang++ -std=c++11 -I.. -fsanitize=fuzzer,address,undefined \
  libfuzzer_parser.cpp -o libfuzzer_parser
mkdir -p corpus
./libfuzzer_parser corpus
```

The CMake target is opt-in because some Clang installations do not ship the
libFuzzer runtime:

```sh
cmake -S .. -B ../out-fuzz \
  -DARIA_CSV_BUILD_FUZZERS=ON \
  -DARIA_CSV_BUILD_LIBFUZZER=ON
cmake --build ../out-fuzz --target aria_csv_libfuzzer
```

Seed the corpus with the existing CSV fixtures:

```sh
cp ../test/data/*.csv corpus/
```

## AFL++

Use AFL++ with sanitizers:

```text
+-------------+     +-------------+     +----------------+
| seed corpus | --> | AFL++       | --> | afl_parser.cpp |
+-------------+     +-------------+     +----------------+
       ^                                      |
       |                                      v
       +------------------------------ findings/crashes
```

```sh
afl-clang-fast++ -std=c++11 -I.. -fsanitize=address,undefined \
  afl_parser.cpp -o afl_parser
mkdir -p corpus findings
cp ../test/data/*.csv corpus/
afl-fuzz -i corpus -o findings -- ./afl_parser
```

## Sanitizer mutation driver

Some toolchains, including Apple Clang, do not ship the libFuzzer runtime. The
deterministic mutation driver is a fallback that still runs randomized parser
inputs under sanitizers:

```text
+----------------+     +----------------+     +----------------------+
| CSV fixtures   | --> | random_driver  | --> | mutated CSV strings  |
+----------------+     +----------------+     +----------------------+
                                                    |
                                                    v
                                           +----------------------+
                                           | CsvParser + sanitizers|
                                           +----------------------+
```

```sh
clang++ -std=c++11 -I.. -fsanitize=address,undefined \
  random_driver.cpp -o random_driver
ARIA_CSV_FUZZ_RUNS=500000 ./random_driver ../test/data/*.csv
```

## Property tests

The property tests use RapidCheck, a C++11 QuickCheck-style framework with
shrinking. They are opt-in so normal unit-test builds stay lightweight:

```sh
cmake -S test -B test/out -DARIA_CSV_ENABLE_PROPERTY_TESTS=ON
cmake --build test/out
./test/out/parser_property_test
```
