#!/usr/bin/env python3
"""Check the actual combinatorics listings against exact counts and enumeration."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LISTING = re.compile(
    r'\\begin\{lstlisting\}\s*(?:\[[^]]*\])?\s*\n(.*?)\\end\{lstlisting\}', re.S)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='clang++')
    parser.add_argument('--cxxflag', action='append', default=[])
    args = parser.parse_args()
    code = '''#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;
using ll = long long;
using i128 = __int128_t;
'''
    # Dependencies first: examples are printed beside the theorem they explain.
    for name in ('math/gcd_and_modular.tex', 'math/combinatorics.tex',
                 'math/essential_theorems.tex'):
        code += '\n'.join(LISTING.findall((ROOT / name).read_text())) + '\n'
    code += r'''
ll exact_choose(int n, int k) {
    if (k < 0 || k > n) return 0;
    i128 answer = 1;
    for (int i = 1; i <= k; ++i) answer = answer * (n - i + 1) / i;
    return (ll)answer;
}

ll factorial(int n) {
    ll answer = 1;
    for (int i = 2; i <= n; ++i) answer *= i;
    return answer;
}

int main() {
    const ll MOD = 1000000007;
    Combinations c(100, MOD);
    for (ll mod : {1LL, 7LL, 12LL, MOD}) {
        auto pascal = pascal_mod(40, mod);
        for (int n = 0; n <= 40; ++n)
            for (int k = 0; k <= 40; ++k)
                assert(pascal[n][k] == exact_choose(n, k) % mod);
    }
    for (int n = 0; n <= 40; ++n)
        for (int k = -1; k <= n + 1; ++k)
            assert(c.choose(n, k) == exact_choose(n, k) % MOD);
    for (int a = 0; a <= 4; ++a)
        for (int b = 0; b <= 4; ++b)
            for (int d = 0; d <= 4; ++d) {
                ll expected = factorial(a+b+d) / factorial(a) / factorial(b) / factorial(d);
                assert(multinomial({a,b,d}, c) == expected % MOD);
            }
    assert(multinomial({}, c) == 1);

    mt19937 rng(20261026);
    for (int trial = 0; trial < 500; ++trial) {
        int n = rng() % 13, k = 1 + rng() % 6;
        vector<int> upper(k);
        for (int &u : upper) u = rng() % 6;
        vector<ll> dp(n + 1);
        dp[0] = 1;
        for (int u : upper) {
            vector<ll> next(n + 1);
            for (int sum = 0; sum <= n; ++sum)
                for (int x = 0; x <= u && sum + x <= n; ++x)
                    next[sum+x] += dp[sum];
            dp.swap(next);
        }
        assert(bounded_distributions(n, upper, c) == dp[n]);
    }
    assert(bounded_distributions(6, {3,3,3}, c) == 10);
    for (int n = 0; n <= 8; ++n) {
        function<ll(int,int)> balanced = [&](int opened, int closed) -> ll {
            if (closed == n) return 1;
            ll answer = 0;
            if (opened < n) answer += balanced(opened+1, closed);
            if (closed < opened) answer += balanced(opened, closed+1);
            return answer;
        };
        assert(catalan(n, c) == balanced(0, 0));
        vector<int> permutation(n);
        iota(permutation.begin(), permutation.end(), 0);
        ll expected = 0;
        do {
            bool valid = true;
            for (int i = 0; i < n; ++i) valid &= permutation[i] != i;
            expected += valid;
        } while (next_permutation(permutation.begin(), permutation.end()));
        for (ll mod : {1LL, 12LL, MOD})
            assert(derangements(n, mod)[n] == expected % mod);
    }
    for (int n = 1; n <= 8; ++n) for (int colors = 1; colors <= 3; ++colors) {
        set<string> representatives;
        function<void(string)> enumerate = [&](string s) {
            if ((int)s.size() < n) {
                for (int i = 0; i < colors; ++i) enumerate(s + char('a'+i));
                return;
            }
            string best = s;
            for (int r = 1; r < n; ++r) best = min(best, s.substr(r) + s.substr(0,r));
            representatives.insert(best);
        };
        enumerate("");
        for (ll mod : {12LL, 25LL, MOD}) if (gcd((ll)n, mod) == 1)
            assert(rotation_colorings(n, colors, mod) == (ll)representatives.size() % mod);
    }
    assert(rotation_colorings(4, 0, MOD) == 0);
    ostringstream output;
    auto previous = cout.rdbuf(output.rdbuf());
    counting_examples();
    cout.rdbuf(previous);
    assert(output.str() == "10\n10000\n1\n");
    cout << "OK: Pascal and factorial combinations, 125 multinomials, 500 bounded distributions, "
            "Catalan vs balanced parentheses, derangements vs permutations, "
            "Burnside vs explicit rotation classes, and printed examples\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='handbook-combinatorics-') as tmp:
        source, executable = Path(tmp) / 'test.cpp', Path(tmp) / 'test'
        source.write_text(code)
        subprocess.run([args.compiler, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                        *args.cxxflag, str(source), '-o', str(executable)], check=True)
        subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    main()
