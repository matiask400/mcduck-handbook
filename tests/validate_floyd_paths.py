#!/usr/bin/env python3
"""Check optional Floyd paths against independent layered Bellman-Ford."""
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
    code = next(b for b in blocks if 'void floyd_warshall(' in b)
    example = next(b for b in blocks if 'void floyd_path_example(' in b)
    uncomment = lambda text: re.sub(r'(?m)^(\s*)// (.*)$', r'\1\2', text)
    base = code.replace('floyd_warshall(', 'floyd_distances_only(').replace(
        'negative_cycle_between(', 'negative_cycle_distances_only(')
    source = r'''
#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <vector>
using namespace std;
using ll = long long;
const ll LINF = numeric_limits<ll>::max() / 4;
'''
    source += base + uncomment(code) + uncomment(example) + r'''
int pairs_checked = 0, paths_checked = 0;
void check(const vector<vector<ll>>& cost) {
    int n = cost.size();
    auto plain = cost, distance = cost;
    vector<vector<int>> next;
    floyd_distances_only(plain);
    floyd_warshall(distance, next);
    assert(distance == plain);
    for (int source = 0; source < n; ++source) {
        vector<ll> best(n, LINF);
        best[source] = 0;
        for (int round = 0; round < n - 1; ++round) {
            auto previous = best;
            for (int u = 0; u < n; ++u) if (previous[u] != LINF)
                for (int v = 0; v < n; ++v) if (cost[u][v] != LINF)
                    best[v] = min(best[v], previous[u] + cost[u][v]);
        }
        vector<char> bad(n, false);
        for (int u = 0; u < n; ++u) if (best[u] != LINF)
            for (int v = 0; v < n; ++v) if (cost[u][v] != LINF)
                if (best[u] + cost[u][v] < best[v]) bad[v] = true;
        for (int round = 0; round < n; ++round)
            for (int u = 0; u < n; ++u) if (bad[u])
                for (int v = 0; v < n; ++v) if (cost[u][v] != LINF) bad[v] = true;
        for (int target = 0; target < n; ++target) {
            assert(negative_cycle_between(distance, source, target) == (bool)bad[target]);
            auto path = floyd_path(distance, next, source, target);
            if (bad[target] || best[target] == LINF) assert(path.empty());
            else {
                assert(distance[source][target] == best[target]);
                assert(!path.empty() && path.front() == source && path.back() == target);
                assert((int)path.size() <= n);
                ll total = 0;
                vector<char> seen(n, false);
                for (int i = 0; i < (int)path.size(); ++i) {
                    int v = path[i];
                    assert(0 <= v && v < n && !seen[v]);
                    seen[v] = true;
                    if (i) {
                        assert(cost[path[i - 1]][v] != LINF);
                        total += cost[path[i - 1]][v];
                    }
                }
                assert(total == best[target]);
                ++paths_checked;
            }
            ++pairs_checked;
        }
    }
}
int main() {
    ostringstream captured;
    auto* previous_output = cout.rdbuf(captured.rdbuf());
    floyd_path_example();
    cout.rdbuf(previous_output);
    assert(captured.str() == "0 1 2 3 ");
    check({}); check({{0}}); check({{-1}});
    vector<vector<ll>> partial_cycle(5, vector<ll>(5, LINF));
    for (int i = 0; i < 5; ++i) partial_cycle[i][i] = 0;
    partial_cycle[0][1] = 2; partial_cycle[1][2] = -3;
    partial_cycle[2][1] = 1; partial_cycle[2][3] = 4;
    check(partial_cycle);
    for (int encoding = 0; encoding < 729; ++encoding) {
        vector<vector<ll>> cost(3, vector<ll>(3, LINF));
        for (int i = 0; i < 3; ++i) cost[i][i] = 0;
        int digits = encoding;
        for (int u = 0; u < 3; ++u) for (int v = 0; v < 3; ++v) if (u != v) {
            int state = digits % 3; digits /= 3;
            if (state) cost[u][v] = state == 1 ? -2 : 3;
        }
        check(cost);
    }
    mt19937_64 rng(20261025);
    for (int trial = 0; trial < 2000; ++trial) {
        int n = 1 + rng() % 8;
        vector<vector<ll>> cost(n, vector<ll>(n, LINF));
        for (int u = 0; u < n; ++u) {
            cost[u][u] = 0;
            for (int v = 0; v < n; ++v) if (u != v && rng() % 3 == 0)
                cost[u][v] = (int)(rng() % 41) - 20;
        }
        check(cost);
    }
    cout << "OK: " << pairs_checked << " source/target pairs; " << paths_checked
         << " finite paths, unreachable targets and affected/unaffected negative cycles\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-floyd-paths-') as directory:
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
