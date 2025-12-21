#!/usr/bin/env python3
from __future__ import annotations

from collections import Counter
import csv
from pathlib import Path


INPUT_FILE = Path("rand_str_input_sec.txt")
OUTPUT_FILE = Path("expected_results.csv")
MAX_LEN = 64


def compute_expected(line: str) -> tuple[str, int]:
    s = line.rstrip("\n")[:MAX_LEN]
    s = s.replace(" ", "")  # Ignore ASCII space (32)

    if not s:
        return ("?", 0)

    counts = Counter(s)
    best_count = max(counts.values())

    # Tie-breaker: smaller ASCII code wins
    best_char = min(
        (ch for ch, n in counts.items() if n == best_count),
        key=lambda ch: ord(ch),
    )

    return best_char, best_count


def main() -> None:
    if not INPUT_FILE.exists():
        raise FileNotFoundError(f"Missing input file: {INPUT_FILE}")

    with INPUT_FILE.open("r", encoding="utf-8", errors="replace") as f, \
         OUTPUT_FILE.open("w", newline="", encoding="utf-8") as out:

        writer = csv.writer(out)
        writer.writerow(["Test #", "Most frequent character", "Count"])

        for idx, line in enumerate(f, start=1):
            ch, count = compute_expected(line)
            writer.writerow([idx, ch, count])

    print(f"Wrote results to {OUTPUT_FILE}")


if __name__ == "__main__":
    main()

