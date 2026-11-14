"""Regression coverage for the listing forms used by the handbook."""
from pathlib import Path
import unittest

from validate_snippets import extract_cpp, extract_python, flatten_tex


class ListingExtractionTests(unittest.TestCase):
    def test_whitespace_before_options(self):
        text = "\\begin{lstlisting} [style=cpp]\nint value = 1;\n\\end{lstlisting}"
        self.assertIn("int value = 1;", extract_cpp(text))

    def test_python_styles_and_language_are_separate(self):
        for options in ("style=py", "style={py}", "language=Python", "title={Python}"):
            with self.subTest(options=options):
                text = f"\\begin{{lstlisting}}[{options}]\nprint(1)\n\\end{{lstlisting}}"
                self.assertIn("print(1)", extract_python(text))
                self.assertEqual(extract_cpp(text), "")

    def test_actual_book_contains_both_languages(self):
        root = Path(__file__).resolve().parents[1]
        flattened = flatten_tex(root, root / "main.tex")
        self.assertIn("#include <bits/stdc++.h>", extract_cpp(flattened))
        self.assertIn("import sys", extract_python(flattened))
        self.assertNotIn("import sys", extract_cpp(flattened))


if __name__ == "__main__":
    unittest.main()
