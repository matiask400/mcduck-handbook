#!/usr/bin/env python3
"""Compile the optional DP, Dinic and HLD recoveries and verify their examples."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LISTING = re.compile(r'\\begin\{lstlisting\}\[style=cpp\]\s*\n(.*?)\\end\{lstlisting\}', re.S)


def blocks(path):
    return LISTING.findall((ROOT / path).read_text())


def uncomment(code):
    return re.sub(r'(?m)^(\s*)// (.*)$', r'\1\2', code)


def library():
    snippets = []
    dp = blocks('dp/knapsack_and_bitmask.tex')
    for name, recovered in (('knapsack_01', 'knapsack_with_items'),
                            ('hamiltonian_path', 'hamiltonian_with_vertices')):
        code = next(b for b in dp if 'll ' + name + '(' in b)
        snippets.extend((code, uncomment(code).replace(name + '(', recovered + '(')))
    dinic = next(b for b in blocks('graphs/flows.tex') if 'struct Dinic {' in b)
    snippets.extend((dinic, re.sub(r'\bDinic\b', 'DinicRecovered', uncomment(dinic))))
    hld = next(b for b in blocks('graphs/heavy_light_decomposition.tex') if 'struct HLD {' in b)
    recovered_hld = re.sub(
        r'(?ms)^    // template<class Callback>\n.*?^    // }',
        lambda match: uncomment(match[0]), hld)
    snippets.extend((hld, re.sub(r'\bHLD\b', 'HLDEdges', recovered_hld)))
    snippets.append(next(b for b in blocks('data_structures/segment_tree.tex')
                         if 'struct SegmentTree {' in b))
    for path, name in (
        ('dp/knapsack_and_bitmask.tex', 'knapsack_recovery_example'),
        ('dp/knapsack_and_bitmask.tex', 'hamiltonian_recovery_example'),
        ('graphs/flows.tex', 'dinic_recovery_example'),
        ('graphs/heavy_light_decomposition.tex', 'hld_edge_example'),
    ):
        example = uncomment(next(b for b in blocks(path) if 'void ' + name + '(' in b))
        example = example.replace('knapsack_01(', 'knapsack_with_items(')
        example = example.replace('hamiltonian_path(', 'hamiltonian_with_vertices(')
        example = re.sub(r'\bDinic\b', 'DinicRecovered', example)
        example = re.sub(r'\bHLD\b', 'HLDEdges', example)
        snippets.append(example)
    return '\n'.join(snippets)


HEADER = r'''
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;
using ll = long long;
const ll LINF = numeric_limits<ll>::max() / 4;
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--cxxflag', action='append', default=[])
    args = parser.parse_args()
    source = HEADER + library() + (ROOT / 'tests/recovery_examples_test.cpp').read_text()
    with tempfile.TemporaryDirectory(prefix='mcduck-recovery-') as directory:
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
