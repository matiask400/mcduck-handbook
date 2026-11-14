#!/usr/bin/env python3
"""Validate optional BFS parents and multiorigin owners from the actual listings."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LISTING = re.compile(r'\\begin\{lstlisting\}\[style=cpp\]\s*\n(.*?)\\end\{lstlisting\}', re.S)


def function(block, name):
    start = block.index('vector<int> ' + name + '(')
    opening = block.index('{', start)
    depth = 0
    for position in range(opening, len(block)):
        depth += (block[position] == '{') - (block[position] == '}')
        if depth == 0:
            return block[start:position + 1]
    raise ValueError('Unclosed function: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--cxxflag', action='append', default=[])
    args = parser.parse_args()
    snippets = []
    for path, name, example in (
        ('graphs/traversal.tex', 'bfs_distances', 'bfs_path_example'),
        ('graphs/multisource_bfs.tex', 'multi_source_bfs', 'multi_source_path_example'),
        ('graphs/shortest_paths.tex', 'zero_one_bfs', 'zero_one_bfs_path_example'),
    ):
        blocks = LISTING.findall((ROOT / path).read_text())
        base = function(next(b for b in blocks if 'vector<int> ' + name + '(' in b), name)
        enable = lambda text: re.sub(r'(?m)^(\s*)// (.*)$', r'\1\2', text)
        snippets.extend((base, enable(base).replace(name + '(', name + '_with_parents(')))
        sample = next(b for b in blocks if 'void ' + example + '(' in b)
        snippets.append(enable(sample).replace(name + '(', name + '_with_parents('))
    source = r'''
#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;
const int INF = numeric_limits<int>::max() / 4;
using Graph = vector<vector<int>>;
using Weighted = vector<vector<pair<int,int>>>;
'''
    source += '\n'.join(snippets) + r'''
long long path_checks = 0, owner_checks = 0;
void check_path(const Weighted& graph, const vector<int>& distance,
                const vector<int>& parent, int target, int root, int unreachable) {
    if (distance[target] == unreachable) { assert(parent[target] == -1); return; }
    int n = graph.size(), steps = 0;
    assert(0 <= root && root < n && distance[root] == 0);
    for (int v = target; v != root; v = parent[v]) {
        assert(++steps < n);
        int from = parent[v];
        assert(0 <= from && from < n);
        bool edge_exists = false;
        for (auto [to, weight] : graph[from])
            if (to == v && distance[from] + weight == distance[v]) edge_exists = true;
        assert(edge_exists);
    }
    assert(parent[root] == -1);
    ++path_checks;
}
void check(const Weighted& graph) {
    int n = graph.size();
    Graph plain(n);
    Weighted unit(n);
    vector<vector<int>> weighted(n, vector<int>(n, INF)), unweighted = weighted;
    for (int v = 0; v < n; ++v) weighted[v][v] = unweighted[v][v] = 0;
    for (int u = 0; u < n; ++u) for (auto [v, w] : graph[u]) {
        plain[u].push_back(v); unit[u].push_back({v, 1});
        weighted[u][v] = min(weighted[u][v], w);
        unweighted[u][v] = min(unweighted[u][v], 1);
    }
    for (auto* matrix : {&weighted, &unweighted})
        for (int k = 0; k < n; ++k)
            for (int i = 0; i < n; ++i) if ((*matrix)[i][k] != INF)
                for (int j = 0; j < n; ++j) if ((*matrix)[k][j] != INF)
                    (*matrix)[i][j] = min((*matrix)[i][j], (*matrix)[i][k] + (*matrix)[k][j]);
    for (int source = 0; source < n; ++source) {
        vector<int> parent(n, 99);
        auto distance = bfs_distances_with_parents(plain, source, parent);
        assert(distance == bfs_distances(plain, source));
        for (int target = 0; target < n; ++target) {
            assert(distance[target] == (unweighted[source][target] == INF ? -1 : unweighted[source][target]));
            check_path(unit, distance, parent, target, source, -1);
        }
        distance = zero_one_bfs_with_parents(graph, source, parent);
        assert(distance == zero_one_bfs(graph, source) && distance == weighted[source]);
        for (int target = 0; target < n; ++target)
            check_path(graph, distance, parent, target, source, INF);
    }
    vector<vector<int>> source_sets{{}};
    if (n) source_sets.insert(source_sets.end(), {{0}, {0, n - 1}, {n - 1, 0}, {0, n - 1, 0}});
    for (const auto& sources : source_sets) {
        vector<int> parent(n, 99), owner(n, 99);
        auto distance = multi_source_bfs_with_parents(plain, sources, parent, owner);
        assert(distance == multi_source_bfs(plain, sources));
        for (int target = 0; target < n; ++target) {
            int expected_distance = INF, expected_owner = -1;
            for (int source : sources) if (unweighted[source][target] < expected_distance) {
                expected_distance = unweighted[source][target];
                expected_owner = source;
            }
            assert(distance[target] == (expected_distance == INF ? -1 : expected_distance));
            assert(owner[target] == expected_owner);
            check_path(unit, distance, parent, target, owner[target], -1);
            ++owner_checks;
        }
    }
}
void check_output(void (*example)(), const string& expected) {
    ostringstream captured;
    auto* previous_output = cout.rdbuf(captured.rdbuf());
    example();
    cout.rdbuf(previous_output);
    assert(captured.str() == expected);
}
int main() {
    check_output(bfs_path_example, "0 1 3 ");
    check_output(multi_source_path_example, "0\n0 1 2 ");
    check_output(zero_one_bfs_path_example, "0 1 2 3 ");
    check({}); check(Weighted(1)); check(Weighted(5));
    check({{{1,1}, {2,0}, {2,1}}, {{2,0}, {3,1}}, {{1,0}}, {}, {}});
    check({{{1,1}}, {{0,1}, {2,1}}, {{1,1}, {3,1}}, {{2,1}, {4,1}}, {{3,1}}});
    mt19937_64 rng(20261025);
    for (int trial = 0; trial < 1000; ++trial) {
        int n = 1 + rng() % 20;
        Weighted graph(n);
        for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
            if (rng() % 5 == 0) {
                graph[u].push_back({v, (int)(rng() & 1)});
                if (rng() % 7 == 0) graph[u].push_back({v, (int)(rng() & 1)});
            }
        check(graph);
    }
    cout << "OK: " << path_checks << " paths and " << owner_checks
         << " nearest-source checks; both variants, examples, ties, empty sources and zero cycles\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-bfs-paths-') as directory:
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
