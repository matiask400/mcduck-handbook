#!/usr/bin/env python3
"""Validate book structure, listing syntax, and the maintained execution suites."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path


INPUT_RE = re.compile(r"\\input\{([^}]+)\}")
LISTING_RE = re.compile(
    r"\\begin\{lstlisting\}\s*(?:\[(?P<options>[^]]*)\])?\s*\n"
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


def is_python_listing(options: str) -> bool:
    return bool(re.search(
        r"(?:language|title)\s*=\s*\{?Python\b|style\s*=\s*\{?py\b",
        options, re.IGNORECASE,
    ))


def extract_cpp(tex: str) -> str:
    snippets = []
    for match in LISTING_RE.finditer(tex):
        options = match.group("options") or ""
        if is_python_listing(options):
            continue
        snippets.append(match.group("code"))
    return "\n\n".join(snippets)


def extract_python(tex: str) -> str:
    snippets = []
    for match in LISTING_RE.finditer(tex):
        options = match.group("options") or ""
        if is_python_listing(options):
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
    parser.add_argument("--cxxflag", action="append", default=[])
    parser.add_argument("--skip-run", action="store_true")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    flattened = flatten_tex(root, root / "main.tex")
    check_environments(flattened)
    check_algorithm_documentation(root)
    for index, match in enumerate(LISTING_RE.finditer(flattened)):
        if is_python_listing(match.group("options") or ""):
            compile(match.group("code"), f"handbook-python-{index}", "exec")

    # Book examples contain local statements and alternative templates. Reuse
    # the audit's definition selection instead of concatenating them globally.
    from audit_book import library
    cpp, _ = library()
    template_text = (root / "core/template_cpp.tex").read_text(encoding="utf-8")
    template = next(LISTING_RE.finditer(template_text)).group("code")
    version = subprocess.check_output([args.compiler, "--version"], text=True)
    extra_flags = args.cxxflag.copy()
    if "clang" in version.lower():
        extra_flags.append("-Wno-unqualified-std-cast-call")
    flags = ["-std=gnu++17", "-Wall", "-Wextra", "-Werror", *extra_flags]

    with tempfile.TemporaryDirectory(prefix="mcduck-handbook-") as temp:
        temp_path = Path(temp)
        for name, source in (("template", template), ("definitions", cpp)):
            path = temp_path / f"{name}.cpp"
            path.write_text(source, encoding="utf-8")
            run([args.compiler, *flags, "-fsyntax-only", str(path)])

    if args.skip_run:
        print("OK: docs, LaTeX inputs, Python syntax and selected C++ definitions")
        return

    suites = [
        ("audit_book.py", ["--case", "correctness"]),
        ("audit_integration.py", ["--with-pbds"]),
        *[(name, []) for name in (
            "validate_bellman_ford.py", "validate_bfs_paths.py",
            "validate_combinatorics.py", "validate_convex_diameter.py",
            "validate_divide_conquer_partition.py", "validate_dp_strings.py",
            "validate_floyd_paths.py", "validate_hamiltonian_path.py",
            "validate_math_helpers.py", "validate_mitm.py",
            "validate_recovery_examples.py", "validate_rmq.py",
            "validate_sa_tarjan.py", "validate_segmented_sieve.py",
            "validate_stl_sums.py", "validate_complexity_examples.py",
            "validate_edge_fixes.py", "validate_new_modules.py",
            "factorization_stress.py",
        )],
    ]
    for name, options in suites:
        command = [sys.executable, str(root / "tests" / name),
                   "--compiler", args.compiler, *options]
        command.extend(f"--cxxflag={flag}" for flag in extra_flags)
        print(f"Checking {name}", flush=True)
        run(command)
    run([sys.executable, "-m", "unittest", "discover", "-s", str(root / "tests"),
         "-p", "test_*.py"])
    print(f"OK: docs, listing syntax, integration and {len(suites)} execution suites")


if __name__ == "__main__":
    main()
