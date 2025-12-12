#!/usr/bin/env python3
"""
export_results_md.py

Convert a CSV experiment log into a Markdown table for easy reporting.
"""

import argparse
import csv
from pathlib import Path


def load_rows(csv_path: Path):
    with csv_path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)
    return rows, reader.fieldnames or []


def write_markdown(rows, headers, output_path: Path):
    if not headers:
        raise ValueError("CSV file must include a header row.")
    with output_path.open("w", encoding="utf-8") as handle:
        handle.write("| " + " | ".join(headers) + " |\n")
        handle.write("| " + " | ".join("---" for _ in headers) + " |\n")
        for row in rows:
            values = [row.get(h, "") or "" for h in headers]
            handle.write("| " + " | ".join(values) + " |\n")


def main():
    parser = argparse.ArgumentParser(description="Convert CSV experiment logs to Markdown tables.")
    parser.add_argument("-i", "--input", required=True, help="Input CSV file path.")
    parser.add_argument("-o", "--output", help="Output Markdown file path (defaults to <input>.md).")
    args = parser.parse_args()

    csv_path = Path(args.input)
    if not csv_path.exists():
        raise SystemExit(f"CSV file not found: {csv_path}")

    rows, headers = load_rows(csv_path)
    if not rows:
        raise SystemExit("CSV file is empty.")

    output_path = Path(args.output) if args.output else csv_path.with_suffix(".md")
    write_markdown(rows, headers, output_path)
    print(f"Wrote {len(rows)} rows to {output_path}")


if __name__ == "__main__":
    main()
