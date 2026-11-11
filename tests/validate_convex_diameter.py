#!/usr/bin/env python3
"""Compare the actual calipers listing with an independent all-pairs oracle."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LISTING = re.compile(
    r'\\begin\{lstlisting\}\s*\[style=cpp\]\s*\n(.*?)\\end\{lstlisting\}', re.S)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--cxxflag', action='append', default=[])
    args = parser.parse_args()
    geometry = LISTING.findall((ROOT / 'geometry/geometry_2d.tex').read_text())
    calipers = LISTING.findall(
        (ROOT / 'geometry/lines_circles_polygons.tex').read_text())
    snippets = '\n'.join(
        next(block for block in blocks if marker in block)
        for blocks, marker in (
            (geometry, 'struct Point {'),
            (geometry, 'vector<Point> convex_hull('),
            (calipers, 'i128 convex_diameter_squared('),
        ))
    source = r'''
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>
using namespace std;
using ll = long long;
using i128 = __int128_t;
'''
    source += snippets + r'''
i128 oracle(const vector<Point>& polygon) {
    i128 answer = 0;
    for (auto a : polygon) for (auto b : polygon) {
        i128 dx = (i128)a.x - b.x, dy = (i128)a.y - b.y;
        answer = max(answer, dx * dx + dy * dy);
    }
    return answer;
}
int checks = 0;
void check(vector<Point> polygon) {
    i128 expected = oracle(polygon);
    int n = polygon.size();
    for (int shift : {0, n / 2, max(0, n - 1)}) {
        auto rotated = polygon;
        if (n) rotate(rotated.begin(), rotated.begin() + shift, rotated.end());
        for (int direction = 0; direction < 2; ++direction) {
            auto original = rotated;
            assert(convex_diameter_squared(rotated) == expected);
            assert(rotated == original); // The input must remain unchanged.
            ++checks;
            reverse(rotated.begin(), rotated.end());
        }
    }
}
int main() {
    check({}); check({{0, 0}}); check({{0, 0}, {3, 4}});
    check({{0, 0}, {1, 1}, {2, 2}, {3, 3}}); // Collinear degeneracy.
    check({{2, 0}, {-3, 0}, {0, 0}, {1, 0}}); // Unsorted collinear points.
    vector<Point> square = {{0, 2}, {2, 2}, {2, 0}, {0, 0}};
    assert(convex_diameter_squared(square) == 8);
    check(square);
    vector<Point> regression = {{1, 9}, {4, 0}, {-6, -9}, {-7, -3}, {-9, 10}};
    assert(convex_diameter_squared(regression) == 373);
    check(regression);
    for (int mask = 0; mask < (1 << 9); ++mask) {
        vector<Point> points;
        for (int k = 0; k < 9; ++k)
            if (mask >> k & 1) points.push_back({k % 3 - 1, k / 3 - 1});
        check(convex_hull(points));
    }
    mt19937_64 rng(20261025);
    for (int trial = 0; trial < 20000; ++trial) {
        vector<Point> points;
        int count = 4 + rng() % 77;
        ll radius = trial % 2 == 0 ? 1000000000LL : 1000LL;
        for (int i = 0; i < count; ++i)
            points.push_back({(ll)(rng() % (2 * radius + 1)) - radius,
                              (ll)(rng() % (2 * radius + 1)) - radius});
        check(convex_hull(points));
    }
    cout << "OK: " << checks << " diameter checks against all pairs; "
         << "both orientations, rotated starts, degeneracies and regressions\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-calipers-') as directory:
        source_path = Path(directory) / 'test.cpp'
        executable = Path(directory) / 'test'
        source_path.write_text(source)
        subprocess.run([
            args.compiler, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
            *args.cxxflag, str(source_path), '-o', str(executable),
        ], check=True)
        subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    main()
