#!/usr/bin/env python3
"""Test the actual listings; optionally benchmark against a saved source tree."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FILES = (
    'techniques/binary_search.tex', 'techniques/two_pointers.tex',
    'strings/aho_corasick.tex', 'graphs/biconnected_components.tex',
    'graphs/virtual_tree.tex', 'data_structures/segment_tree.tex',
)
LISTING = re.compile(
    r'\\begin\{lstlisting\}\s*(?:\[[^]]*\])?\s*\n(.*?)\\end\{lstlisting\}', re.S)
HEADER = '''#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
using ll = long long;
using i128 = __int128_t;
constexpr ll LINF = numeric_limits<ll>::max() / 4;
'''


def listings(root, name):
    if isinstance(root, Path):
        source = (root / name).read_text()
    else:
        source = subprocess.check_output(
            ['git', 'show', f'{root}:{name}'], cwd=ROOT, text=True)
    return LISTING.findall(source)


def wrap_real(code, name):
    return (f'double {name}(double start, double end, double threshold, int &calls) {{\n'
            'auto can = [&](double x) { ++calls; return x * x >= threshold; };\n'
            + code + '\nreturn r;\n}\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='clang++')
    parser.add_argument('--cxxflag', action='append', default=[])
    baseline_group = parser.add_mutually_exclusive_group()
    baseline_group.add_argument('--baseline', type=Path,
                                help='Saved source tree; enables benchmarks and line counts')
    baseline_group.add_argument('--baseline-ref', help='Git revision to benchmark against')
    args = parser.parse_args()
    baseline = args.baseline or args.baseline_ref
    binary = listings(ROOT, FILES[0])
    code = HEADER
    code += ('int first_true_example(int start, int end, int threshold) {\n'
             'auto can = [&](int x) { return x >= threshold; };\n'
             + binary[0] + '\nreturn l;\n}\n')
    code += wrap_real(binary[1], 'real_search')
    for name in FILES[1:]:
        code += listings(ROOT, name)[0].split('\n// Uso', 1)[0] + '\n'
    segment = listings(ROOT, FILES[-1])[0]
    variants = {
        'Min': ('LINF', 'min(a, b)'),
        'Max': ('-LINF', 'max(a, b)'),
        'Gcd': ('0', 'gcd(a, b)'),
        'Xor': ('0', 'a ^ b'),
    }
    for name, (neutral, operation) in variants.items():
        code += (segment.replace('SegmentTree', 'SegmentTree' + name)
                 .replace('NEUTRAL = 0', 'NEUTRAL = ' + neutral)
                 .replace('return a + b;', 'return ' + operation + ';'))
    if baseline:
        code += '\n#define HAVE_BASELINE\n'
        code += wrap_real(listings(baseline, FILES[0])[1], 'old_real_search')
        for namespace, name in (('before_aho', FILES[2]), ('before_segment', FILES[-1])):
            code += '\nnamespace ' + namespace + ' {\n'
            code += listings(baseline, name)[0] + '\n}\n'
    code += (ROOT / 'tests/complexity_examples_test.cpp').read_text()
    with tempfile.TemporaryDirectory(prefix='handbook-complexities-') as tmp:
        source, executable = Path(tmp) / 'test.cpp', Path(tmp) / 'test'
        source.write_text(code)
        warnings = ['-Wno-unqualified-std-cast-call'] if 'clang' in args.compiler else []
        subprocess.run([args.compiler, '-std=c++17', '-O2', '-Wall', '-Wextra',
                        '-Werror', *warnings, *args.cxxflag, str(source), '-o', str(executable)],
                       check=True)
        subprocess.run([str(executable)], check=True)
    if baseline:
        print('Listing lines (including comments and blank lines):', flush=True)
        for name in FILES:
            before = sum(len(s.splitlines()) for s in listings(baseline, name))
            after = sum(len(s.splitlines()) for s in listings(ROOT, name))
            print(f'{name}: {before} -> {after} ({after - before:+d})')


if __name__ == '__main__':
    main()
