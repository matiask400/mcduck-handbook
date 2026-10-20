#!/usr/bin/env python3
"""Compile the actual sieve listings and compare segmented primes to independent oracles."""
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
    tex = (ROOT / 'math/sieve_and_factorization.tex').read_text()
    snippets = '\n'.join(re.findall(
        r'\\begin\{lstlisting\}\[style=cpp\]\s*\n(.*?)\\end\{lstlisting\}', tex, re.S))
    harness = r'''
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
using namespace std;
using ll = long long;
'''
    harness += snippets + r'''
int main() {
    const int N = 1002000;
    vector<bool> oracle(N + 1, true);
    oracle[0] = oracle[1] = false;
    for (int p = 2; p <= N / p; ++p)
        if (oracle[p]) for (int x = p * p; x <= N; x += p) oracle[x] = false;
    vector<int> primes;
    for (int n = 2; n <= N; ++n) if (oracle[n]) primes.push_back(n);
    int checks = 0;
    auto check_small = [&](int L, int R) {
        vector<ll> expected;
        for (int n = L; n <= R; ++n) if (oracle[n]) expected.push_back(n);
        assert(segmented_primes(L, R) == expected);
        ++checks;
    };
    for (int L = 0; L <= 200; ++L)
        for (int R = L; R <= 200; ++R) check_small(L, R);
    mt19937_64 rng(20261016);
    for (int i = 0; i < 2000; ++i) {
        int L = rng() % 1000000, R = L + rng() % 1001;
        check_small(L, R);
    }
    auto trial_prime = [&](ll n) {
        if (n < 2) return false;
        for (ll p : primes) {
            if (p > n / p) break;
            if (n % p == 0) return false;
        }
        return true;
    };
    auto check_large = [&](ll L, ll R) {
        vector<ll> expected;
        for (ll n = L; n <= R; ++n)
            if (trial_prime(n)) expected.push_back(n);
        assert(segmented_primes(L, R) == expected);
        ++checks;
    };
    for (int i = 0; i < 30; ++i) {
        ll L = 1000000000000LL - 100000 + (ll)(rng() % 200001);
        check_large(L, L + 30);
    }
    for (ll p : {2LL, 3LL, 97LL, 997LL, 999983LL}) {
        ll square = p * p;
        check_large(square - 1, square + 1);
        check_large(square, square);
    }
    check_small(0, 1000000); // Wide interval, including 0, 1 and every prime itself.
    assert(segmented_primes(0, 0).empty());
    assert(segmented_primes(1, 1).empty());
    assert(segmented_primes(2, 2) == vector<ll>{2});
    assert(segmented_primes(100, 120) == vector<ll>({101, 103, 107, 109, 113}));
    cout << "OK: " << checks << " interval comparisons plus boundary examples; "
         << "exhaustive [0,200], random intervals, prime squares, "
         << "width 1000001 and values near 10^12\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-segmented-sieve-') as directory:
        source, exe = Path(directory) / 'test.cpp', Path(directory) / 'test'
        source.write_text(harness)
        subprocess.run([args.compiler, '-std=gnu++17', '-O2', '-Wall', '-Wextra',
                        '-Wshadow', '-Werror', *args.cxxflag, str(source), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    main()
