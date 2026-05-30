#!/usr/bin/env python3
import argparse
import csv
import os
import subprocess
import sys
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
BENCHMARK = ROOT / "benchmark"
OUT = Path(os.environ.get("ARIA_CSV_BENCH_OUT", "/tmp/aria_csv_benchmark"))
CASES = {
    "plain-small": "/tmp/aria_csv_plain_small.csv",
    "plain-large": "/tmp/aria_csv_plain_large.csv",
    "quoted": "/tmp/aria_csv_quoted.csv",
    "wide": "/tmp/aria_csv_wide.csv",
    "large-fields": "/tmp/aria_csv_large_fields.csv",
}


def run(command, cwd=ROOT):
    return subprocess.run(
        command,
        cwd=str(cwd),
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def build_optimize(cxx, cxxflags):
    OUT.mkdir(parents=True, exist_ok=True)
    binary = OUT / "aria_csv_optimize"
    command = [
        cxx,
        "-std=c++11",
        *cxxflags,
        "-I.",
        "benchmark/optimize.cpp",
        "-o",
        str(binary),
    ]
    run(command)
    return binary


def run_optimize(binary, iterations):
    result = run([str(binary), str(iterations)])
    rows = list(csv.DictReader(result.stdout.splitlines()))
    return rows


def print_generated_results(rows):
    print("Generated workload benchmark")
    print()
    print("| workload | mode | bytes | iterations | best ms | MiB/s | checksum |")
    print("| --- | --- | ---: | ---: | ---: | ---: | ---: |")
    for row in rows:
      print(
          "| {workload} | {mode} | {bytes} | {iterations} | {best_ms} | "
          "{mb_per_s} | {checksum} |".format(**row)
      )


def generate_file(path, case, kind, rows, cols, field_size):
    command = [
        sys.executable,
        "benchmark/generate_csv.py",
        str(path),
        "--case",
        case,
    ]
    if kind is not None:
        command += ["--kind", kind]
    if rows is not None:
        command += ["--rows", str(rows)]
    if cols is not None:
        command += ["--cols", str(cols)]
    if field_size is not None:
        command += ["--field-size", str(field_size)]
    result = run(command)
    return result.stdout.strip()


def build_external():
    cpp = BENCHMARK / "cpp" / "bench.out"
    run(
        [
            "clang++",
            "-std=c++11",
            "-O2",
            "-DNDEBUG",
            "-I../..",
            "main.cpp",
            "-o",
            "bench.out",
        ],
        cwd=BENCHMARK / "cpp",
    )

    rust = BENCHMARK / "rust" / "target" / "release" / "bench"
    run(["cargo", "build", "--release"], cwd=BENCHMARK / "rust")

    node_modules = BENCHMARK / "js" / "node_modules"
    if not node_modules.exists():
        run(["npm", "install"], cwd=BENCHMARK / "js")

    return cpp, rust


def time_command(command, iterations):
    best_elapsed = None
    best_count = ""
    for _ in range(iterations):
        count, elapsed = time_command_once(command)
        if best_elapsed is None or elapsed < best_elapsed:
            best_elapsed = elapsed
            best_count = count
    return best_count, best_elapsed


def time_command_once(command):
    start = time.perf_counter()
    result = subprocess.run(
        command,
        cwd=str(BENCHMARK),
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    elapsed = time.perf_counter() - start

    rows = result.stdout.strip().splitlines()
    count = rows[-1] if rows else ""
    return count, elapsed


def print_external_results(path, iterations):
    cpp, rust = build_external()
    sample = Path(path)
    sample_for_command = str(sample)
    sample_for_stat = sample if sample.is_absolute() else BENCHMARK / sample
    cases = [
        ("Aria CSV parser", [str(cpp.relative_to(BENCHMARK)), sample_for_command]),
        ("Rust csv 1.4.0", [str(rust.relative_to(BENCHMARK)), sample_for_command]),
        ("Node csv-parser 3.0.0", ["node", "js/main.js", sample_for_command]),
    ]
    bytes_read = sample_for_stat.stat().st_size

    print()
    print("Comparative file benchmark")
    print()
    print("| parser | rows | best seconds | MiB/s |")
    print("| --- | ---: | ---: | ---: |")
    for name, command in cases:
        count, elapsed = time_command(command, iterations)
        mib_per_s = 0.0 if elapsed == 0.0 else bytes_read / elapsed / (1024.0 * 1024.0)
        print(f"| {name} | {count} | {elapsed:.3f} | {mib_per_s:.2f} |")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--iterations", type=int, default=7)
    parser.add_argument("--cxx", default=os.environ.get("CXX", "clang++"))
    parser.add_argument(
        "--cxxflag",
        action="append",
        default=["-O2", "-DNDEBUG", "-g", "-fno-omit-frame-pointer"],
        help="extra C++ compiler flag; may be passed multiple times",
    )
    parser.add_argument(
        "--external",
        action="store_true",
        help="run the C++/Rust/Node comparison",
    )
    parser.add_argument("--file", type=Path, help="CSV file for --external")
    parser.add_argument(
        "--generate",
        type=Path,
        help="generate a CSV file at this path before --external",
    )
    parser.add_argument(
        "--case",
        choices=sorted(CASES),
        help="named generated CSV case for --external",
    )
    parser.add_argument(
        "--kind",
        choices=["plain", "quoted", "wide", "huge-fields"],
        help="override generated CSV shape",
    )
    parser.add_argument("--rows", type=int)
    parser.add_argument("--cols", type=int)
    parser.add_argument("--field-size", type=int)
    args = parser.parse_args()

    if args.iterations <= 0:
        print("--iterations must be positive", file=sys.stderr)
        return 2

    binary = build_optimize(args.cxx, args.cxxflag)
    rows = run_optimize(binary, args.iterations)
    print_generated_results(rows)

    file_path = args.file
    if args.case is not None and args.generate is None:
        args.generate = Path(CASES[args.case])
    if args.generate is not None:
        case = args.case if args.case is not None else "plain"
        kind = args.kind if args.kind is not None else None
        print()
        print(generate_file(args.generate, case, kind, args.rows, args.cols,
                            args.field_size))
        file_path = args.generate

    if args.external:
        external_path = file_path
        if external_path is None:
            external_path = Path("sample.csv")
        print_external_results(external_path, args.iterations)

    return 0


if __name__ == "__main__":
    sys.exit(main())
