#!/usr/bin/env python3
"""Compute line statistics (total lines) for files under ./main distinguishing generated vs manual.

Heuristics for 'generated':
  - Filenames ending with _gen.c or _gen.h
  - Paths containing /fonts/ and ending with _ttf_data.c
  - Paths containing /images/ and ending with _data.c
  - file_list_gen.cmake
  - Any file whose first 5 lines contain phrases like 'Automatically generated' or 'Do not edit'

Outputs JSON with per-file classification and aggregate totals.

Limitations:
  - Cannot confidently distinguish AI-authored vs human-authored code; treats anything not matching generated heuristics as 'manual'.
  - Lines counted are physical lines including blanks.
"""
from __future__ import annotations
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MAIN = ROOT / "main"

GENERATED_PATTERNS = [
    re.compile(r"_gen\.(c|h)$"),
    re.compile(r"file_list_gen\.cmake$"),
    re.compile(r"_ttf_data\.c$"),
    re.compile(r"_bg_data\.c$"),
    re.compile(r"_data\.c$"),
]

HEADER_MARKERS = [
    "Automatically generated",
    "Auto-generated",
    "DO NOT EDIT",
    "Do not edit",
]

def is_generated(path: Path) -> bool:
    name = str(path).replace("\\", "/")
    for pat in GENERATED_PATTERNS:
        if pat.search(name):
            return True
    # Inspect first few lines for generation markers
    try:
        with path.open('r', errors='ignore') as f:
            for _ in range(5):
                line = f.readline()
                if not line:
                    break
                if any(marker in line for marker in HEADER_MARKERS):
                    return True
    except Exception:
        pass
    return False

def classify_files():
    files = []
    for ext in (".c", ".h", ".cmake"):
        for p in MAIN.rglob(f"*{ext}"):
            if not p.is_file():
                continue
            try:
                with p.open('r', errors='ignore') as f:
                    lines = sum(1 for _ in f)
            except Exception:
                lines = 0
            gen = is_generated(p)
            files.append({
                "path": str(p.relative_to(ROOT)),
                "lines": lines,
                "generated": gen,
            })
    return files

def aggregate(files):
    total_lines = sum(f["lines"] for f in files)
    gen_lines = sum(f["lines"] for f in files if f["generated"])
    manual_lines = total_lines - gen_lines
    return {
        "total_lines": total_lines,
        "generated_lines": gen_lines,
        "manual_lines": manual_lines,
        "generated_pct": round(gen_lines / total_lines * 100, 2) if total_lines else 0.0,
        "manual_pct": round(manual_lines / total_lines * 100, 2) if total_lines else 0.0,
    }

def main():
    files = classify_files()
    agg = aggregate(files)
    out = {"summary": agg, "files": files, "heuristic_notes": [
        "Classification is heuristic; AI-authored manual code cannot be distinguished.",
        "Generated detection based on filename patterns and header markers.",
    ]}
    print(json.dumps(out, indent=2))

if __name__ == "__main__":
    main()
