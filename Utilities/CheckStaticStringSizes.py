#!/usr/bin/env python3
"""
Scan the project for StaticString size usages and suggest reusing larger sizes.

Finds:
  - StaticString::Data<N>
  - StaticString::Container(std::integral_constant<size_t, N>{...})
  - StaticString::Container("string")  (string literal; size = decoded length)

Reports when a size is used only once and a larger size exists elsewhere.
The recommendation is that you *could* switch to the larger size to share the
static buffer, but use judgment: coalescing many strings to one size may
increase simultaneous use of that slot and lead to more dynamic allocations.

Usage:
  python3 check_static_string_sizes.py [--root /path/to/repo] [path1 path2 ...]
  If no paths given, scans from repo root (or --root). Paths can be files or dirs.
"""

import argparse
import re
import sys
from pathlib import Path
from collections import defaultdict

# Match StaticString::Data< NUMBER >
DATA_PATTERN = re.compile(
    r"StaticString::Data\s*<\s*(\d+)\s*>",
    re.MULTILINE,
)

# Match integral_constant<size_t, NUMBER> (with optional std::)
INTEGRAL_CONST_PATTERN = re.compile(
    r"(?:std::)?integral_constant\s*<\s*size_t\s*,\s*(\d+)\s*>",
    re.MULTILINE,
)

# Match StaticString::Container("...") - string literal; group(1) = content between quotes
CONTAINER_LITERAL_PATTERN = re.compile(
    r'StaticString::Container\s*\(\s*"((?:[^"\\]|\\.)*)"\s*\)',
    re.MULTILINE,
)


def decoded_string_literal_length(content: str) -> int:
    """Return the length in bytes of a C++ string literal content (between the quotes)."""
    length = 0
    i = 0
    while i < len(content):
        if content[i] == "\\" and i + 1 < len(content):
            nxt = content[i + 1]
            if nxt in "\\\"'?nrt0":
                length += 1
                i += 2
                continue
            if nxt == "x" and i + 3 < len(content):
                length += 1
                i += 4
                continue
            if nxt in "01234567":
                length += 1
                j = i + 2
                while j < len(content) and j - (i + 1) < 3 and content[j] in "01234567":
                    j += 1
                i = j
                continue
        length += 1
        i += 1
    return length


def scan_file(path: Path) -> list[tuple[int, int, str]]:
    """Return list of (size, line_no, line_text) for path."""
    results = []
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return results
    for line_no, line in enumerate(text.splitlines(), start=1):
        for pattern in (DATA_PATTERN, INTEGRAL_CONST_PATTERN):
            for m in pattern.finditer(line):
                results.append((int(m.group(1)), line_no, line.strip()))
        for m in CONTAINER_LITERAL_PATTERN.finditer(line):
            results.append((decoded_string_literal_length(m.group(1)), line_no, line.strip()))
    return results


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Suggest StaticString size reuse across the project."
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parent.parent.parent,
        help="Repo root to scan (default: parent of tools/StaticStringSizeCheck)",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        help="Files or dirs to scan (default: root, excluding common non-source dirs)",
    )
    parser.add_argument(
        "--extensions",
        default=".cpp,.hpp,.h,.cxx,.hxx",
        help="Comma-separated extensions to include (default: .cpp,.hpp,.h,.cxx,.hxx)",
    )
    parser.add_argument(
        "--exclude",
        default="vendors,build,.git",
        help="Comma-separated path segments to exclude (default: vendors,build,.git)",
    )
    parser.add_argument(
        "--list-files",
        action="store_true",
        help="For each size, list the files where it was found (relative paths)",
    )
    args = parser.parse_args()
    root = args.root.resolve()
    exts = set(args.extensions.split(","))
    exclude = set(args.exclude.split(","))

    def should_scan(p: Path) -> bool:
        if p.suffix not in exts:
            return False
        try:
            rel = p.resolve().relative_to(root)
        except ValueError:
            return False
        parts = rel.parts
        return not any(x in exclude for x in parts)

    if args.paths:
        to_scan: list[Path] = []
        for p in args.paths:
            p = p.resolve()
            if p.is_file():
                if should_scan(p):
                    to_scan.append(p)
            else:
                to_scan.extend(f for f in p.rglob("*") if f.is_file() and should_scan(f))
    else:
        to_scan = [f for f in root.rglob("*") if f.is_file() and should_scan(f)]

    # (size, file, line_no, line_text)
    all_uses: list[tuple[int, Path, int, str]] = []
    for path in sorted(to_scan):
        for size, line_no, line_text in scan_file(path):
            all_uses.append((size, path, line_no, line_text))

    # size -> list of (file, line_no, line_text)
    by_size: dict[int, list[tuple[Path, int, str]]] = defaultdict(list)
    for size, path, line_no, line_text in all_uses:
        by_size[size].append((path, line_no, line_text))

    sizes_sorted = sorted(by_size.keys())
    print(f"Scanned {len(to_scan)} files (root: {root})", file=sys.stderr)
    if not by_size:
        print("No StaticString sizes found (Data<N>, integral_constant<size_t, N>, or Container(\"...\")).", file=sys.stderr)
    else:
        print("Sizes found:", file=sys.stderr)
        for s in sizes_sorted:
            occurrences = by_size[s]
            count = len(occurrences)
            if args.list_files:
                seen: set[str] = set()
                for path, _, _ in occurrences:
                    try:
                        seen.add(str(path.resolve().relative_to(root)))
                    except ValueError:
                        seen.add(str(path))
                files_str = ", ".join(sorted(seen))
                print(f"  {s} bytes: {count} use(s) -- {files_str}", file=sys.stderr)
            else:
                print(f"  {s} bytes: {count} use(s)", file=sys.stderr)

    exit_code = 0
    for size in sizes_sorted:
        occurrences = by_size[size]
        if len(occurrences) > 1:
            continue
        # This size is unique; see if a larger size exists
        larger = [s for s in sizes_sorted if s > size]
        if not larger:
            continue
        next_larger = min(larger)
        for path, line_no, line_text in occurrences:
            try:
                rel = path.relative_to(root)
            except ValueError:
                rel = path
            print(
                f"{rel}:{line_no}: size {size} is unique; you could use size {next_larger} (used "
                f"elsewhere) to share the static buffer. Use judgment: coalescing to one size may "
                f"increase simultaneous use of that slot and lead to more dynamic allocations. ",
                file=sys.stderr,
            )
            print(f"  {line_text[:80]}{'...' if len(line_text) > 80 else ''}", file=sys.stderr)
            exit_code = 1
    if exit_code == 0 and by_size:
        print(
            "No suggestions: every size is either used more than once (not unique) or "
            "is the largest (no larger size to suggest). You may wish to coalesce string "
            "sizes together to save memory if their usage patterns (such as threading) would "
            "not cause contention on the static buffer and force a dynamic allocation.",
            file=sys.stderr,
        )
    return exit_code


if __name__ == "__main__":
    sys.exit(main())