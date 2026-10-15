#!/usr/bin/env python3
"""Validate LaTeX structure and compile/run the handbook's C++ snippets."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path


INPUT_RE = re.compile(r"\\input\{([^}]+)\}")
LISTING_RE = re.compile(
    r"\\begin\{lstlisting\}(?:\[(?P<options>[^]]*)\])?\s*\n"
    r"(?P<code>.*?)\\end\{lstlisting\}",
    re.DOTALL,
)


def resolve_input(root: Path, name: str) -> Path:
    path = root / name
    if path.suffix == "":
        path = path.with_suffix(".tex")
    return path


def flatten_tex(root: Path, path: Path, stack: tuple[Path, ...] = ()) -> str:
    path = path.resolve()
    if path in stack:
        cycle = " -> ".join(str(item) for item in (*stack, path))
        raise RuntimeError(f"circular input: {cycle}")
    text = path.read_text(encoding="utf-8")

    def replace(match: re.Match[str]) -> str:
        child = resolve_input(root, match.group(1))
        if not child.exists():
            raise FileNotFoundError(f"missing input {match.group(1)!r} from {path}")
        return flatten_tex(root, child, (*stack, path))

    return INPUT_RE.sub(replace, text)


def check_environments(tex: str) -> None:
    without_listings = LISTING_RE.sub("", tex)
    for environment in ("document", "itemize", "enumerate", "center", "tabular"):
        starts = len(re.findall(rf"\\begin\{{{environment}\}}", without_listings))
        ends = len(re.findall(rf"\\end\{{{environment}\}}", without_listings))
        if starts != ends:
            raise RuntimeError(
                f"unbalanced {environment}: {starts} begin, {ends} end"
            )


def check_algorithm_documentation(root: Path) -> None:
    """Every algorithm section must explain recognition and resource costs."""
    algorithm_folders = (
        "data_structures", "dp", "geometry", "graphs", "math", "strings",
        "techniques",
    )
    for folder in algorithm_folders:
        for path in sorted((root / folder).glob("*.tex")):
            text = path.read_text(encoding="utf-8")
            sections = len(re.findall(r"\\section\{", text))
            use_cases = len(re.findall(r"\\whenuse\{", text))
            complexities = len(re.findall(r"\\complexity\{", text))
            memory_mentions = len(re.findall(r"memoria", text, re.IGNORECASE))
            if use_cases != sections:
                raise RuntimeError(
                    f"{path}: {sections} sections but {use_cases} use cases"
                )
            if complexities < sections or memory_mentions < sections:
                raise RuntimeError(
                    f"{path}: every section needs time and memory complexity"
                )


def extract_cpp(tex: str) -> str:
    snippets = []
    for match in LISTING_RE.finditer(tex):
        options = match.group("options") or ""
        if re.search(r"(?:language|title)\s*=\s*\{?Python", options, re.IGNORECASE):
            continue
        snippets.append(match.group("code"))
    return "\n\n".join(snippets)


def extract_python(tex: str) -> str:
    snippets = []
    for match in LISTING_RE.finditer(tex):
        options = match.group("options") or ""
        if re.search(r"(?:language|title)\s*=\s*\{?Python", options, re.IGNORECASE):
            snippets.append(match.group("code"))
    return "\n\n".join(snippets)


def run(command: list[str], **kwargs: object) -> None:
    completed = subprocess.run(command, text=True, capture_output=True, **kwargs)
    if completed.returncode != 0:
        if completed.stdout:
            print(completed.stdout)
        if completed.stderr:
            print(completed.stderr)
        raise SystemExit(completed.returncode)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--skip-run", action="store_true")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    flattened = flatten_tex(root, root / "main.tex")
    check_environments(flattened)
    check_algorithm_documentation(root)
    cpp = extract_cpp(flattened)
    python = extract_python(flattened)
    smoke_tests = (root / "tests" / "smoke_tests.cpp").read_text(encoding="utf-8")

    with tempfile.TemporaryDirectory(prefix="mcduck-handbook-") as temp:
        temp_path = Path(temp)
        syntax_source = temp_path / "syntax.cpp"
        syntax_source.write_text(cpp, encoding="utf-8")
        run([
            args.compiler, "-std=gnu++17", "-Wall", "-Wextra", "-Wshadow",
            "-Werror", "-fsyntax-only", str(syntax_source),
        ])

        if python:
            python_source = temp_path / "syntax.py"
            python_source.write_text(python, encoding="utf-8")
            run([sys.executable, "-m", "py_compile", str(python_source)])

        if not args.skip_run:
            test_source = temp_path / "tests.cpp"
            test_source.write_text(
                "#define main handbook_template_main\n"
                + cpp
                + "\n#undef main\n"
                + smoke_tests,
                encoding="utf-8",
            )
            executable = temp_path / "tests"
            run([
                args.compiler, "-std=gnu++17", "-O2", "-Wall", "-Wextra",
                "-Wshadow", "-Werror", str(test_source), "-o", str(executable),
            ])
            run([str(executable)])

    print("OK: docs, LaTeX inputs, C++/Python syntax and C++ smoke tests")


if __name__ == "__main__":
    main()
