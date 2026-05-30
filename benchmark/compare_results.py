#!/usr/bin/env python3
import csv
import sys


def read_results(path):
    with open(path, newline="") as handle:
        rows = {}
        for row in csv.DictReader(handle):
            key = (row["workload"], row["mode"])
            rows[key] = {
                "mb_per_s": float(row["mb_per_s"]),
                "best_ms": float(row["best_ms"]),
                "checksum": row["checksum"],
            }
        return rows


def main():
    if len(sys.argv) != 3:
        print("usage: compare_results.py <baseline.csv> <after.csv>",
              file=sys.stderr)
        return 2

    baseline = read_results(sys.argv[1])
    after = read_results(sys.argv[2])
    keys = sorted(set(baseline) | set(after))

    print("| workload | mode | baseline MB/s | after MB/s | delta | checksum |")
    print("| --- | --- | ---: | ---: | ---: | --- |")
    failed = False
    for key in keys:
        if key not in baseline or key not in after:
            print("| {} | {} | missing | missing | missing | missing |".format(
                key[0], key[1]))
            failed = True
            continue

        base = baseline[key]
        new = after[key]
        delta = ((new["mb_per_s"] / base["mb_per_s"]) - 1.0) * 100.0
        checksum = "ok" if base["checksum"] == new["checksum"] else "changed"
        if checksum != "ok":
            failed = True

        print("| {} | {} | {:.2f} | {:.2f} | {:+.2f}% | {} |".format(
            key[0], key[1], base["mb_per_s"], new["mb_per_s"], delta,
            checksum))

    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
