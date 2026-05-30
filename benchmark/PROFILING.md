# Profiling

The profiling driver keeps file I/O out of the hot path by reading the CSV once
and reparsing the in-memory contents repeatedly. Build it with frame pointers and
debug symbols so sampling profilers can produce useful call trees.

## Shape

```text
+-------------+     +----------------+     +----------------------+
| sample.csv  | --> | read once      | --> | in-memory CSV text   |
+-------------+     +----------------+     +----------------------+
                                                   |
                                                   v
                                          +----------------------+
                                          | parse many times     |
                                          | profiler samples CPU |
                                          +----------------------+
```

This keeps disk I/O out of the hot path. The profile should mostly show parser
work, string work, and stream construction around each parse iteration.

## macOS sample

```sh
clang++ -std=c++11 -O2 -g -fno-omit-frame-pointer \
  benchmark/profile.cpp -I. -o /tmp/aria_csv_profile

/tmp/aria_csv_profile benchmark/sample.csv 100000 &
pid=$!
sample "$pid" 5 1 -file /tmp/aria_csv_profile.sample.txt
wait "$pid"
```

The `sample` output is a call-tree profile that can be folded into flamegraph
input with stackcollapse tools, or inspected directly for the hottest parser
frames.

## Linux perf

```sh
clang++ -std=c++11 -O2 -g -fno-omit-frame-pointer \
  benchmark/profile.cpp -I. -o /tmp/aria_csv_profile

perf record -F 997 -g -- /tmp/aria_csv_profile benchmark/sample.csv 100000
perf report
```

For SVG flamegraphs, run Brendan Gregg's `stackcollapse-perf.pl` and
`flamegraph.pl` on `perf script` output.
