#!/usr/bin/env python3
"""Structural check for GitHub Markdown math blocks in the MTP files."""
import re
import sys
from pathlib import Path

FILES = [Path("docs/MTP_REPORT.md"), Path("README.md")]
BANNED = [
    r"\operatorname", r"\bigl", r"\bigr", r"\middle",
    r"\left\{", r"\right\}", r"\left|", r"\right|",
    r"\begin{aligned}", r"\tag{",
]
FENCES = (chr(96) * 3, chr(126) * 3)

def braces_balanced(value):
    depth = 0
    for index, char in enumerate(value):
        if char not in "{}":
            continue
        escapes = 0
        j = index - 1
        while j >= 0 and value[j] == "\\":
            escapes += 1
            j -= 1
        if escapes % 2:
            continue
        depth += 1 if char == "{" else -1
        if depth < 0:
            return False
    return depth == 0

def check(path):
    issues = []
    blocks = 0
    current_marker = None
    current_language = ""
    math_lines = []
    start_line = 0
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        stripped = line.strip()
        marker = next((m for m in FENCES if stripped.startswith(m)), None)
        if marker is not None:
            suffix = stripped[len(marker):].strip()
            if current_marker is None:
                current_marker = marker
                current_language = suffix
                start_line = line_number
                math_lines = []
            elif marker == current_marker and not suffix:
                if current_language == "math":
                    blocks += 1
                    content = "\n".join(math_lines)
                    if not content.strip() or not braces_balanced(content):
                        issues.append(f"{path}:{start_line}: empty block or unbalanced braces")
                current_marker = None
                current_language = ""
            continue

        if current_marker is not None:
            if current_language == "math":
                math_lines.append(line)
                if "$" in line or len(line) > 100:
                    issues.append(f"{path}:{line_number}: bad/oversized math expression")
                for macro in BANNED:
                    if macro in line:
                        issues.append(f"{path}:{line_number}: unsupported math macro {macro}")
            continue

        if stripped in ("$", "$$"):
            issues.append(f"{path}:{line_number}: bare dollar-only math line; use fenced math")
        if "$$" in line and chr(96) not in line:
            issues.append(f"{path}:{line_number}: use fenced math, not double dollars")
    if current_marker is not None:
        issues.append(f"{path}:{start_line}: unclosed code/math fence")
    return issues, blocks

def main():
    total = 0
    all_issues = []
    for path in FILES:
        issues, n = check(path)
        total += n
        all_issues.extend(issues)
    if total < 20:
        all_issues.append(f"Expected at least 20 display math blocks, got {total}")
    if all_issues:
        print("\n".join(all_issues))
        return 1
    print(f"Markdown math check passed: {total} fenced math blocks.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
