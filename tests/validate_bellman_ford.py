#!/usr/bin/env python3
"""Compare Bellman-Ford with independent 128-bit Floyd-Warshall on small graphs."""
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
        (ROOT / 'graphs/shortest_paths.tex').read_text(), re.S)
    code = next(b for b in blocks if 'bellman_ford(' in b)
    output_example = next(b for b in blocks if 'void bellman_ford_output_example(' in b)
    reconstruction, count = re.subn(
        r'(?m)^(\s*)// (.*// Reconstruccion:.*)$', r'\1\2',
        code.split('// Ejemplo opcional:', 1)[0])
    assert count == 3
    reconstruction = reconstruction.replace('bellman_ford(', 'bellman_ford_with_parents(')
    reconstruction = re.sub(r'^struct WeightedEdge[^\n]*\n', '', reconstruction, flags=re.M)
    source = r'''
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
using namespace std;
using ll = long long;
using i128 = __int128_t;
const ll LINF = numeric_limits<ll>::max() / 4;
'''
    source += code + reconstruction + output_example + r'''
int checks = 0, path_checks = 0;
void check(int n, const vector<WeightedEdge>& edges, int source) {
    const i128 BIG = (i128)1 << 126;
    vector<vector<i128>> d(n, vector<i128>(n, BIG));
    for (int v = 0; v < n; ++v) d[v][v] = 0;
    for (auto e : edges) d[e.from][e.to] = min(d[e.from][e.to], (i128)e.weight);
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i) if (d[i][k] != BIG)
            for (int j = 0; j < n; ++j) if (d[k][j] != BIG)
                d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
    bool cycle = false, unrepresentable = false;
    for (int v = 0; v < n; ++v) {
        if (d[source][v] != BIG && d[v][v] < 0) cycle = true;
        if (d[source][v] != BIG &&
            (d[source][v] < numeric_limits<ll>::min() ||
             d[source][v] > numeric_limits<ll>::max() || d[source][v] == LINF))
            unrepresentable = true;
    }
    for (bool reconstruct : {false, true}) {
        try {
            vector<int> parent;
            auto [distance, negative_cycle] = reconstruct
                ? bellman_ford_with_parents(n, edges, source, parent)
                : bellman_ford(n, edges, source);
            assert(negative_cycle == cycle);
            if (cycle) assert(distance.empty());
            else {
                assert(!unrepresentable && (int)distance.size() == n);
                for (int v = 0; v < n; ++v)
                    assert(distance[v] == (d[source][v] == BIG ? LINF : (ll)d[source][v]));
                if (reconstruct) for (int target = 0; target < n; ++target) {
                    if (distance[target] == LINF) { assert(parent[target] == -1); continue; }
                    vector<char> seen(n, false);
                    for (int v = target; v != source; v = parent[v]) {
                        assert(0 <= v && v < n && !seen[v]);
                        seen[v] = true;
                        int from = parent[v];
                        assert(0 <= from && from < n && distance[from] != LINF);
                        bool valid_edge = false;
                        for (auto e : edges)
                            if (e.from == from && e.to == v &&
                                (i128)distance[from] + e.weight == distance[v]) valid_edge = true;
                        assert(valid_edge);
                    }
                    assert(parent[source] == -1);
                    ++path_checks;
                }
            }
        } catch (const overflow_error&) {
            assert(!cycle && unrepresentable);
        }
    }
    ++checks;
}
int main() {
    ostringstream captured;
    auto* previous_output = cout.rdbuf(captured.rdbuf());
    bellman_ford_output_example();
    cout.rdbuf(previous_output);
    assert(captured.str() == "overflow\n");
    const ll lo = numeric_limits<ll>::min(), hi = numeric_limits<ll>::max();
    check(1, {}, 0);
    check(3, {{0, 1, -LINF + 1}, {1, 2, -2}}, 0); // Saturation false positive.
    auto regression = bellman_ford(3, {{0, 1, -LINF + 1}, {1, 2, -2}}, 0);
    assert(!regression.second && regression.first[2] == -LINF - 1);
    check(2, {{0, 1, hi}}, 0);
    check(2, {{0, 1, lo}}, 0);
    check(2, {{0, 1, LINF}}, 0); // A finite result cannot use the unreachable mark.
    check(3, {{0, 1, hi}, {1, 2, hi}}, 0);
    check(3, {{0, 1, lo}, {1, 2, -1}}, 0);
    check(3, {{0, 1, hi}, {1, 2, hi}, {0, 2, -1}}, 0);
    check(1, {{0, 0, lo}}, 0); // Cycle detected before narrowing any distance.
    check(3, {{1, 2, -1}, {2, 1, -1}}, 0); // Unreachable negative cycle.
    check(3, {{0, 1, 0}, {1, 2, -1}, {2, 1, 0}}, 0);
    for (int encoding = 0; encoding < 729; ++encoding) {
        vector<WeightedEdge> edges;
        int digits = encoding;
        for (int u = 0; u < 3; ++u) for (int v = 0; v < 3; ++v) if (u != v) {
            int state = digits % 3;
            digits /= 3;
            if (state) edges.push_back({u, v, state == 1 ? -3 : 5});
        }
        for (int source = 0; source < 3; ++source) check(3, edges, source);
    }
    mt19937_64 rng(20261025);
    for (int trial = 0; trial < 2000; ++trial) {
        int n = 1 + rng() % 8;
        vector<WeightedEdge> edges;
        for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
            if (rng() % 4 == 0) edges.push_back({u, v, (int)(rng() % 101) - 50});
        if (!edges.empty() && trial % 5 == 0) edges.push_back(edges.front());
        shuffle(edges.begin(), edges.end(), rng);
        for (int source = 0; source < n; ++source) check(n, edges, source);
    }
    cout << "OK: " << checks << " source/graph checks in both variants; "
         << path_checks << " reconstructed paths checked against 128-bit Floyd\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-bellman-') as directory:
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
