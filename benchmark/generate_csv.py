#!/usr/bin/env python3
import argparse
from pathlib import Path

CASES = {
    "plain-small": {
        "kind": "plain",
        "rows": 100000,
        "cols": 12,
        "field_size": 262144,
    },
    "plain-large": {
        "kind": "plain",
        "rows": 1000000,
        "cols": 12,
        "field_size": 262144,
    },
    "quoted": {
        "kind": "quoted",
        "rows": 250000,
        "cols": 8,
        "field_size": 262144,
    },
    "wide": {"kind": "wide", "rows": 100000, "cols": 200, "field_size": 262144},
    "large-fields": {
        "kind": "huge-fields",
        "rows": 1000,
        "cols": 2,
        "field_size": 65536,
    },
}


def write_plain(handle, rows, cols):
    for row in range(rows):
        values = [f"field{col % 10}_{row % 100}" for col in range(cols)]
        handle.write(",".join(values))
        handle.write("\n")


def write_quoted(handle, rows, cols):
    for row in range(rows):
        values = []
        for col in range(cols):
            if col % 3 == 0:
                values.append(f'"alpha,beta,{row % 100}"')
            elif col % 3 == 1:
                values.append(f'"said ""hello {col % 10}"""')
            else:
                values.append(f'"line {row % 10}\\nnext"')
        handle.write(",".join(values))
        handle.write("\n")


def write_wide(handle, rows, cols):
    for row in range(rows):
        values = [chr(ord("a") + (col % 26)) for col in range(cols)]
        handle.write(",".join(values))
        handle.write("\n")


def write_huge_fields(handle, rows, cols, field_size):
    field = "x" * field_size
    for _ in range(rows):
        values = [field for _ in range(cols)]
        handle.write(",".join(values))
        handle.write("\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("output", type=Path)
    parser.add_argument("--case", choices=sorted(CASES))
    parser.add_argument("--kind", choices=["plain", "quoted", "wide", "huge-fields"])
    parser.add_argument("--rows", type=int)
    parser.add_argument("--cols", type=int)
    parser.add_argument(
        "--field-size",
        type=int,
        help="field size for huge-fields",
    )
    args = parser.parse_args()

    config = CASES.get(args.case, CASES["plain-small"]).copy()
    if args.kind is not None:
        config["kind"] = args.kind
    if args.rows is not None:
        config["rows"] = args.rows
    if args.cols is not None:
        config["cols"] = args.cols
    if args.field_size is not None:
        config["field_size"] = args.field_size

    if config["rows"] < 0 or config["cols"] <= 0 or config["field_size"] <= 0:
        parser.error("rows must be non-negative; cols and field-size must be positive")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as handle:
        if config["kind"] == "plain":
            write_plain(handle, config["rows"], config["cols"])
        elif config["kind"] == "quoted":
            write_quoted(handle, config["rows"], config["cols"])
        elif config["kind"] == "wide":
            write_wide(handle, config["rows"], config["cols"])
        else:
            write_huge_fields(handle, config["rows"], config["cols"],
                              config["field_size"])

    print(
        "wrote {path} kind={kind} rows={rows} cols={cols} bytes={bytes}".format(
            path=args.output,
            kind=config["kind"],
            rows=config["rows"],
            cols=config["cols"],
            bytes=args.output.stat().st_size,
        )
    )


if __name__ == "__main__":
    main()
