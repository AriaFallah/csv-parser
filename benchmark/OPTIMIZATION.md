# Optimization Workflow

The goal is to make the parser faster without letting the implementation turn
into clever, brittle code. Every optimization should carry both a speed result
and a readability/maintenance result.

## Loop

```text
+-------------+     +-------------+     +-------------+     +-------------+
| baseline    | --> | code change | --> | benchmark   | --> | decision    |
| numbers     |     | small/local |     | same inputs |     | keep/revert |
+-------------+     +-------------+     +-------------+     +-------------+
       ^                                                        |
       |                                                        |
       +---------------- tests, fuzz smoke, readability --------+
```

The benchmark is evidence for a change, not a replacement for tests.

## Baseline

Build the optimization harness with release flags:

```sh
python3 benchmark/run.py --iterations 7
```

For raw CSV output:

```sh
clang++ -std=c++11 -O2 -DNDEBUG -g -fno-omit-frame-pointer \
  benchmark/optimize.cpp -I. -o /tmp/aria_csv_optimize
/tmp/aria_csv_optimize 7 | tee benchmark/baseline.csv
```

The harness prints CSV:

```text
workload,mode,bytes,iterations,best_ms,mb_per_s,checksum
```

Use the best time across iterations to reduce scheduler noise. Re-run the
baseline before comparing a change if the machine load changed noticeably.

## Workloads

The harness generates four in-memory workloads:

```text
+--------------+     +--------------+     +----------------------+
| workload gen | --> | CsvParser    | --> | checksum + timing    |
+--------------+     +--------------+     +----------------------+
| plain        |     | fields mode  |     | behavior guard       |
| quoted       |     | rows mode    |     | speed comparison     |
| wide         |     +--------------+     +----------------------+
| huge-fields  |
+--------------+
```

- `plain`: common unquoted rows with many ordinary fields.
- `quoted`: quoted fields, escaped quotes, commas, and embedded newlines.
- `wide`: many small columns per row.
- `huge-fields`: fields larger than the parser's input buffer.

Each workload runs through both public APIs:

- `fields`: direct `next_field()` parsing.
- `rows`: range iteration over rows.

## Change Gate

For each optimization, record:

- the exact code change
- baseline and after numbers
- percent speed change per workload
- test/fuzz/property status
- readability score
- decision: keep, revise, or revert

Suggested readability score:

- `1`: simpler than before
- `2`: same complexity
- `3`: slightly more complex but local and well named
- `4`: noticeably harder to maintain
- `5`: too clever for this project

Default policy: keep only changes that improve at least one target workload by
5% or more, do not regress any major workload by more than 2%, and score `3` or
better on readability.

Compare two runs with:

```sh
python3 benchmark/compare_results.py benchmark/baseline.csv benchmark/after.csv
```

The comparison script also checks that workload checksums did not change. A
checksum change means the parser behavior changed or the benchmark changed, so
the result is not a pure performance comparison.

## Comparative Gate

When an optimization affects buffering, stream ownership, or row iteration, run
a generated comparative benchmark too:

```sh
python3 benchmark/run.py \
  --iterations 5 \
  --generate /tmp/aria_csv_wide.csv \
  --kind wide \
  --rows 100000 \
  --cols 200 \
  --external
```

The comparative benchmark reports row counts and throughput for Aria, Rust
`csv`, and Node `csv-parser` on the same generated file. Matching row counts are
the first sanity check; throughput is only meaningful after correctness matches.

## Verification

Before keeping a performance change, run:

```sh
cmake -S test -B /tmp/aria_csv_test
cmake --build /tmp/aria_csv_test --parallel
/tmp/aria_csv_test/parser_test

cmake -S test -B /tmp/aria_csv_property -DARIA_CSV_ENABLE_PROPERTY_TESTS=ON
cmake --build /tmp/aria_csv_property --parallel
/tmp/aria_csv_property/parser_property_test

clang++ -std=c++11 -Wall -Wextra -pedantic -I. \
  -fsanitize=fuzzer-no-link,address,undefined \
  -c fuzz/libfuzzer_parser.cpp -o /tmp/aria_csv_libfuzzer.o
```

Run the sampling profiler when a change meaningfully moves benchmark numbers:

```sh
clang++ -std=c++11 -O2 -g -fno-omit-frame-pointer \
  benchmark/profile.cpp -I. -o /tmp/aria_csv_profile

/tmp/aria_csv_profile benchmark/sample.csv 8000 &
pid=$!
sample "$pid" 3 1 -file /tmp/aria_csv_profile.sample.txt
wait "$pid"
```

## Decision Record Template

```md
## YYYY-MM-DD: <change name>

Hypothesis:

Change:

Benchmark:

| workload | mode | baseline MB/s | after MB/s | delta |
| --- | --- | ---: | ---: | ---: |

Verification:

Readability score:

Decision:
```
