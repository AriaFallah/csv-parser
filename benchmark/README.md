# Benchmark

Completely non-scientific benchmark comparing my parser to the fastest node parser I know of
run over `sample.csv`, which has 36635 rows and 18 columns:

#### `C++` results using my parser:
```
time ./main.o sample.csv
done!
        0.14 real         0.14 user         0.00 sys
```

#### `node.js` results using `csv-parser`:

```
$ time node main.js sample.csv
done!
        0.29 real         0.28 user         0.01 sys
```

Will make this benchmarking a lot more legit in the future.
