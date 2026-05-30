# Benchmark

## Current Results

Local comparative runs on generated benchmark cases:

```text
+--------------+-----------------+-----------+---------------+
| case         | parser          | rows      | MiB/s         |
+--------------+-----------------+-----------+---------------+
| plain-small  | Aria CSV parser | 100,000   | 513.50        |
| plain-small  | Rust csv 1.4.0  | 100,000   | 655.14        |
| plain-small  | Node csv-parser | 100,000   | 58.79         |
| large-fields | Aria CSV parser | 1,000     | 1486.18       |
| large-fields | Rust csv 1.4.0  | 1,000     | 2074.60       |
| large-fields | Node csv-parser | 1,000     | 140.53        |
+--------------+-----------------+-----------+---------------+
```

These are process-level timings, so they include file I/O and process startup.
Use the commands below to reproduce on your machine.

There are two benchmark modes.

```text
+----------------------+     +----------------------+
| Internal benchmark   |     | Comparative benchmark|
+----------------------+     +----------------------+
| Aria only            |     | Aria / Rust / Node   |
| generated in memory  |     | generated CSV file   |
| checksum guarded     |     | row-count guarded    |
+----------------------+     +----------------------+
```

## Comparative

Use this when asking “how do we compare to other parsers?”

```sh
python3 benchmark/run.py --case plain-small --iterations 5 --external
python3 benchmark/run.py --case plain-large --iterations 5 --external
python3 benchmark/run.py --case quoted --iterations 5 --external
python3 benchmark/run.py --case wide --iterations 5 --external
python3 benchmark/run.py --case large-fields --iterations 5 --external
```

Named cases are deterministic:

```text
+--------------+------------------------------+
| case         | shape                        |
+--------------+------------------------------+
| plain-small  | 100k rows, 12 columns        |
| plain-large  | 1M rows, 12 columns          |
| quoted       | 250k rows, 8 quoted columns  |
| wide         | 100k rows, 200 columns       |
| large-fields | 1k rows, 2 x 64KiB fields    |
+--------------+------------------------------+
```

Generated files are written under `/tmp` by default and are not committed.

Output:

```text
| parser | rows | best seconds | MiB/s |
```

Rows should match before throughput is taken seriously.

## Internal

Use this when asking “did this parser change help?”

```sh
python3 benchmark/run.py --iterations 7
```

This runs `benchmark/optimize.cpp`, which parses generated in-memory workloads
through both public APIs:

```text
+------------+     +----------------+     +----------------+
| workload   | --> | fields / rows  | --> | time + checksum|
+------------+     +----------------+     +----------------+
```

Output:

```text
| workload | mode | bytes | iterations | best ms | MiB/s | checksum |
```

Checksum changes mean the benchmark behavior changed; investigate before
trusting the speed result.

## Custom Files

Benchmark an existing CSV against all parsers:

```sh
python3 benchmark/run.py --file /path/to/data.csv --iterations 5 --external
```

Generate a custom case:

```sh
python3 benchmark/generate_csv.py /tmp/custom.csv \
  --kind plain \
  --rows 1000000 \
  --cols 12
```

Then compare:

```sh
python3 benchmark/run.py --file /tmp/custom.csv --iterations 5 --external
```

## Notes

The comparative benchmark is process-level. It includes file I/O and program
startup, so use larger generated files when comparing parsers.

The internal benchmark is better for tight parser optimization work because it
keeps data in memory and checks parser behavior with checksums.
