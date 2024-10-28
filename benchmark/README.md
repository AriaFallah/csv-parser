# Benchmark

Parses `sample.csv`, and outputs the number of rows.

#### My parser (c++)
Compiled with `clang++ -std=c++11 -O2`
```
$ time ./bench.out sample.csv
36635

________________________________________________________
Executed in   42.11 millis    fish           external
   usr time   30.07 millis    0.08 millis   29.99 millis
   sys time    7.14 millis    2.98 millis    4.15 millis
```

#### csv 1.3.0 (rust)
Compiled with `cargo build --release`
```
$ time ./rust/target/release/bench sample.csv
36634

________________________________________________________
Executed in   24.54 millis    fish           external
   usr time   16.08 millis    0.06 millis   16.03 millis
   sys time    6.18 millis    2.32 millis    3.86 millis
```

#### csv-parser 3.0.0 (node.js)
```
$ time node js/main.js sample.csv
36634

________________________________________________________
Executed in  194.38 millis    fish           external
   usr time  187.77 millis    0.07 millis  187.70 millis
   sys time   18.24 millis    2.35 millis   15.89 millis
```
