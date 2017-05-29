# Benchmark

Parses `sample.csv`, and outputs the number of rows.

#### My parser (c++)
Compiled with `clang++ -std=c++11 -O2`
```
$ time ./bench.out sample.csv
36634
        0.04 real         0.04 user         0.00 sys
```

#### rust-csv (rust)
Compiled with `cargo build --release`
```
$ time ./rust/target/release/bench sample.csv
36634
        0.02 real         0.01 user         0.00 sys
```

#### csv-parser (node.js)
```
$ time node js/main.js sample.csv
36634
        0.42 real         0.39 user         0.03 sys
```
