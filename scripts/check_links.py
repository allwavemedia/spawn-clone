#!/usr/bin/env python3
"""
Simple Markdown link checker for this repo.

- Scans Markdown files (default: docs/ and top-level README.md if present)
- Validates local relative links resolve to existing files
- Skips external links by default (enable with --external)
- Ignores anchors (links starting with '#') and mailto:
- Exits with non-zero status if any broken links are found

Usage:
  python3 scripts/check_links.py
  python3 scripts/check_links.py --external
  python3 scripts/check_links.py --paths docs docs/workflows

Notes:
- For links with fragments (e.g., file.md#section), only the file existence is checked.
- Links pointing to a directory are considered valid if the directory exists (Markdown often links to folders).
"""

import argparse
import os
import re
import sys
import urllib.parse
import urllib.request

MD_LINK_RE = re.compile(r'\[([^\]]+)\]\(([^)]+)\)')
DEFAULT_PATHS = ["docs"]

def is_external(href: str) -> bool:
    return href.startswith("http://") or href.startswith("https://")

def is_mailto(href: str) -> bool:
    return href.startswith("mailto:")

def is_anchor(href: str) -> bool:
    return href.startswith("#")

def strip_fragment(href: str) -> str:
    # Remove URL fragment (e.g., file.md#anchor)
    return href.split("#", 1)[0]

def check_external(href: str, timeout: int = 5) -> tuple[bool, str]:
    try:
        req = urllib.request.Request(href, method="HEAD")
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            ok = 200 <= resp.status < 400
            return ok, f"HTTP {resp.status}"
    except Exception as e:
        return False, f"Error: {e}"

def find_markdown_files(paths: list[str]) -> list[str]:
    md_files: list[str] = []
    for base in paths:
        if not os.path.exists(base):
            continue
        if os.path.isfile(base) and base.endswith(".md"):
            md_files.append(base)
            continue
        for root, dirs, files in os.walk(base):
            # Skip archive directory from validation (archived/superseded)
            if "docs/archive" in os.path.join(root).replace("\\", "/"):
                continue
            for f in files:
                if f.lower().endswith(".md"):
                    md_files.append(os.path.join(root, f))
    # Add top-level README if present and not already included
    if os.path.exists("README.md"):
        md_files.append("README.md")
    return sorted(set(md_files))

def resolve_local(base_file: str, href: str) -> str:
    target = strip_fragment(href)
    # Handle URL-encoded paths
    target = urllib.parse.unquote(target)
    # Normalize to filesystem path (relative to the markdown file)
    return os.path.normpath(os.path.join(os.path.dirname(base_file), target))

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--external", action="store_true", help="Also check external http(s) links")
    parser.add_argument("--paths", nargs="*", default=DEFAULT_PATHS, help="Paths/files to scan (default: docs)")
    args = parser.parse_args()

    md_files = find_markdown_files(args.paths)
    broken: list[tuple[str, int, str, str]] = []  # (file, line_no, href, reason)
    checked_count = 0

    for md in md_files:
        try:
            with open(md, "r", encoding="utf-8") as fh:
                in_code_block = False
                for lineno, line in enumerate(fh, start=1):
                    stripped = line.lstrip()
                    # Toggle code fence state for triple backticks and skip lines inside fenced code blocks
                    if stripped.startswith("```"):
                        in_code_block = not in_code_block
                        continue
                    if in_code_block:
                        continue
                    # Remove inline code segments `...` before scanning for markdown links
                    scan_line = re.sub(r'`[^`]*`', '', line)
                    for match in MD_LINK_RE.finditer(scan_line):
                        href = match.group(2).strip()
                        # Skip anchors and mailto
                        if is_anchor(href) or is_mailto(href):
                            continue
                        # Skip images (handled via preceding '!' in markdown, but regex doesn't capture it)
                        # Heuristic: if previous char was '!' then it's likely an image. We can't easily
                        # detect it here, so rely on file existence check below (images are files anyway).
                        if is_external(href):
                            if not args.external:
                                continue
                            ok, reason = check_external(href)
                            checked_count += 1
                            if not ok:
                                broken.append((md, lineno, href, reason))
                            continue
                        # Local link
                        if href == "":
                            continue
                        local_path = resolve_local(md, href)
                        checked_count += 1
                        if os.path.exists(local_path):
                            continue
                        # If it doesn't exist as a file, consider valid if it is an existing directory
                        if os.path.isdir(local_path):
                            continue
                        # Also allow directory/index.md convention if linking to a directory-like path without trailing slash
                        if not local_path.lower().endswith(".md"):
                            dir_candidate = local_path
                            if os.path.isdir(dir_candidate) and os.path.exists(os.path.join(dir_candidate, "index.md")):
                                continue
                        # Mark as broken
                        broken.append((md, lineno, href, "Local path not found"))
        except Exception as e:
            broken.append((md, 0, "", f"Read error: {e}"))

    # Report
    if broken:
        print("Broken links detected:")
        for file, lineno, href, reason in broken:
            loc = f"{file}:{lineno}" if lineno else file
            print(f"- {loc} -> {href} ({reason})")
        print(f"\nChecked {checked_count} links. Broken: {len(broken)}")
        return 2
    else:
        print(f"All links OK. Checked {checked_count} links across {len(md_files)} markdown files.")
        return 0

if __name__ == "__main__":
    sys.exit(main())
