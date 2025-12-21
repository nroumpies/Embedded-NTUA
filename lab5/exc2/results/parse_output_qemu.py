import re
import csv
from pathlib import Path

# Input: paste the entire host output into this file OR set INPUT_PATH to a file
INPUT_PATH = "output_qemu.txt"
OUTPUT_CSV = "results_qemu.csv"

RAW_TEXT = r"""
PASTE YOUR ENTIRE OUTPUT HERE
"""

PATTERN = re.compile(
    r'The most frequent character is\s*\n"(.{1})"\s*\nand it appeared (\d+) times\.',
    re.MULTILINE
)

def load_text() -> str:
    if INPUT_PATH is None:
        if "PASTE YOUR ENTIRE OUTPUT HERE" in RAW_TEXT:
            raise ValueError("Replace RAW_TEXT with your pasted output, or set INPUT_PATH.")
        return RAW_TEXT
    return Path(INPUT_PATH).read_text(encoding="utf-8", errors="replace")

def main() -> None:
    text = load_text()
    matches = PATTERN.findall(text)

    if not matches:
        raise ValueError("No matches found. Make sure you pasted the full output.")

    rows = []
    for i, (ch, count) in enumerate(matches, start=1):
        rows.append({
            "Test #": i,
            "Most frequent character": ch,
            "Count": int(count),
        })

    with open(OUTPUT_CSV, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=["Test #", "Most frequent character", "Count"])
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {len(rows)} rows to {OUTPUT_CSV}")

if __name__ == "__main__":
    main()

