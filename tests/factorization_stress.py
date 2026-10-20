#!/usr/bin/env python3
"""Extract and stress-test the number theory listings (no external Python packages)."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LISTING = re.compile(r'\\begin\{lstlisting\}\[style=cpp\]\s*\n(.*?)\\end\{lstlisting\}', re.S)


def extract(path):
    return '\n'.join(LISTING.findall(path.read_text()))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--cxxflag', action='append', default=[])
    parser.add_argument('--seed', type=int, action='append', default=[])
    parser.add_argument('--benchmark', action='store_true')
    parser.add_argument('--baseline', type=Path, help='Previous .tex listing for comparison')
    args = parser.parse_args()
    # Fixed seeds only in the generated test translation unit; book code is unchanged.
    def seeded(code):
        return re.sub(r'chrono::steady_clock::now\(\)\s*\.time_since_epoch\(\).count\(\)',
                      'factorization_test_seed', code)
    header = '''#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
using namespace std;
using ll = long long;
using u128 = __uint128_t;
uint64_t factorization_test_seed;
'''
    code = header
    if args.baseline:
        code += '#define HAVE_FACTORIZATION_BASELINE\nnamespace baseline {\n'
        code += seeded(extract(args.baseline)) + '\n}\n'
    for path in ['math/sieve_and_factorization.tex', 'math/miller_rabin_pollard_rho.tex']:
        code += seeded(extract(ROOT / path)) + '\n'
    code += (ROOT / 'tests/factorization_stress.cpp').read_text()
    with tempfile.TemporaryDirectory(prefix='mcduck-factorization-') as directory:
        source, exe = Path(directory) / 'test.cpp', Path(directory) / 'test'
        source.write_text(code)
        subprocess.run([args.compiler, '-std=gnu++17', '-O2', '-Wall', '-Wextra',
                        '-Wshadow', '-Werror', *args.cxxflag, str(source), '-o', str(exe)], check=True)
        for seed in args.seed or [20261016]:
            subprocess.run([str(exe), str(seed), str(int(args.benchmark))], check=True)


if __name__ == '__main__':
    main()
