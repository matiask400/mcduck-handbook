#!/usr/bin/env python3
"""Check the book's partial_sum input type and operation against 128-bit sums."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--cxxflag', action='append', default=[])
    args = parser.parse_args()
    blocks = re.findall(
        r'\\begin\{lstlisting\}\[style=cpp\]\s*\n(.*?)\\end\{lstlisting\}',
        (ROOT / 'core/stl_algorithms.tex').read_text(), re.S)
    example = next(b for b in blocks if 'void sequence_algorithms()' in b)
    input_type = re.search(r'vector<(\w+)> values\(5\);', example)[1]
    operation = re.search(r'partial_sum\([^;]+;', example)[0]
    source = r'''
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <vector>
using namespace std;
using ll = long long;
using i128 = __int128_t;
'''
    source += example + '\nvoid check(vector<' + input_type + '> values) {\n'
    source += '    vector<ll> prefix(values.size());\n    ' + operation + r'''
    i128 expected = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        expected += values[i];
        assert(expected >= numeric_limits<ll>::min() && expected <= numeric_limits<ll>::max());
        assert(prefix[i] == expected);
    }
}
int main() {
    sequence_algorithms();
    check({}); check({0}); check({10, 11, 12, 13, 14});
    check({2000000000LL, 2000000000LL});
    check({-2000000000LL, -2000000000LL});
    ll lo = numeric_limits<ll>::min(), hi = numeric_limits<ll>::max();
    check({hi, -hi, lo, hi});
    mt19937_64 rng(20261025);
    for (int trial = 0; trial < 10000; ++trial) {
        vector<ll> values(rng() % 101);
        for (ll& v : values) v = (ll)(rng() % 20000000001ULL) - 10000000000LL;
        check(values);
    }
    cout << "OK: partial_sum example, overflow regressions, empty input and 10000 random arrays\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-stl-sums-') as directory:
        path = Path(directory) / 'test.cpp'
        executable = Path(directory) / 'test'
        path.write_text(source)
        subprocess.run([
            args.compiler, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
            *args.cxxflag, str(path), '-o', str(executable),
        ], check=True)
        subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    main()
