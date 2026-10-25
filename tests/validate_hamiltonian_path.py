#!/usr/bin/env python3
"""Compare the Hamiltonian-path listing to exhaustive vertex permutations."""
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
        (ROOT / 'dp/knapsack_and_bitmask.tex').read_text(), re.S)
    code = next(b for b in blocks if 'll hamiltonian_path(' in b)
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
const ll LINF = numeric_limits<ll>::max() / 4;
'''
    source += code + r'''
ll oracle(const vector<vector<ll>>& cost) {
    int n = cost.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    ll answer = LINF;
    do {
        ll total = 0;
        bool valid = true;
        for (int i = 1; i < n; ++i) {
            ll weight = cost[order[i - 1]][order[i]];
            if (weight == LINF) { valid = false; break; }
            total += weight;
        }
        if (valid) answer = min(answer, total);
    } while (next_permutation(order.begin(), order.end()));
    return answer;
}
int checks = 0;
void check(const vector<vector<ll>>& cost) {
    assert(hamiltonian_path(cost) == oracle(cost));
    ++checks;
}
int main() {
    check({{LINF}});
    vector<vector<ll>> regression(3, vector<ll>(3, LINF));
    regression[0][1] = -5; // A negative edge cannot reach the isolated vertex.
    assert(hamiltonian_path(regression) == LINF);
    check(regression);
    regression[1][2] = 2;
    assert(hamiltonian_path(regression) == -3);
    check(regression);
    for (int n = 2; n <= 8; ++n)
        check(vector<vector<ll>>(n, vector<ll>(n, LINF)));
    ll large = (LINF - 1) / 2;
    check({{LINF, large, LINF}, {LINF, LINF, large}, {LINF, LINF, LINF}});
    check({{LINF, -large, LINF}, {LINF, LINF, -large}, {LINF, LINF, LINF}});
    // Exhaust every directed 3-vertex graph with absent, negative or positive edges.
    for (int encoding = 0; encoding < 729; ++encoding) {
        vector<vector<ll>> cost(3, vector<ll>(3, LINF));
        int digits = encoding;
        for (int u = 0; u < 3; ++u) for (int v = 0; v < 3; ++v) if (u != v) {
            int state = digits % 3;
            digits /= 3;
            if (state) cost[u][v] = state == 1 ? -5 : 3;
        }
        check(cost);
    }
    mt19937_64 rng(20261025);
    for (int trial = 0; trial < 2000; ++trial) {
        int n = 1 + rng() % 8;
        vector<vector<ll>> cost(n, vector<ll>(n, LINF));
        int density = trial % 4 == 0 ? 100 : trial % 4 == 1 ? 15 : 50;
        for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
            if (u != v && (int)(rng() % 100) < density)
                cost[u][v] = (int)(rng() % 101) - 50;
        check(cost);
    }
    cout << "OK: " << checks << " matrices checked against all permutations; "
         << "missing edges, negative/zero costs, disconnected and complete graphs\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-hamiltonian-') as directory:
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
