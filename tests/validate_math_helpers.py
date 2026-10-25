#!/usr/bin/env python3
"""Check exact math helpers and power-of-two examples extracted from the book."""
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
        r'\\begin\{lstlisting\}\s*\[style=cpp\]\s*\n(.*?)\\end\{lstlisting\}',
        (ROOT / 'core/math_cpp.tex').read_text(), re.S)
    snippets = '\n'.join(next(b for b in blocks if marker in b) for marker in (
        'll integer_sqrt(', 'll x = 1LL << k;'))
    source = r'''
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
using namespace std;
using ll = long long;
using i128 = __int128_t;
using u128 = __uint128_t;
'''
    source += snippets + r'''
const ll MIN_LL = numeric_limits<ll>::min();
const ll MAX_LL = numeric_limits<ll>::max();
ll sqrt_oracle(ll n) {
    ll lo = 0, hi = 3037000499LL;
    while (lo < hi) {
        ll mid = lo + (hi - lo + 1) / 2;
        if ((i128)mid * mid <= n) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}
void check_division(ll a, ll b) {
    if (b == 0 || (a == MIN_LL && b == -1)) return;
    i128 aa = a, bb = b;
    if (bb < 0) aa = -aa, bb = -bb;
    i128 f = floor_division(a, b), c = ceil_division(a, b);
    assert(f * bb <= aa && aa < (f + 1) * bb);
    assert((c - 1) * bb < aa && aa <= c * bb);
}
int main() {
    assert(binary_power(4294967296LL, 1) == 4294967296LL);
    assert(binary_power(MAX_LL, 1) == MAX_LL);
    assert(binary_power(MIN_LL, 1) == MIN_LL);
    assert(binary_power(-2LL, 63) == MIN_LL);
    assert(binary_power(0LL, 0) == 1);
    assert(binary_power(-1LL, MAX_LL) == -1);
    for (ll base = -12; base <= 12; ++base) {
        i128 expected = 1;
        for (ll exponent = 0; exponent <= 63; ++exponent) {
            if (expected < MIN_LL || expected > MAX_LL) break;
            assert(binary_power(base, exponent) == expected);
            expected *= base;
        }
    }
    for (ll a : {MIN_LL, MIN_LL + 1, -7LL, 0LL, 7LL, MAX_LL})
        for (ll b : {MIN_LL, -3LL, -1LL, 1LL, 3LL, MAX_LL})
            check_division(a, b);
    mt19937_64 rng(20261025);
    for (int trial = 0; trial < 100000; ++trial) {
        ll n = rng() & (unsigned long long)MAX_LL;
        assert(integer_sqrt(n) == sqrt_oracle(n));
        ll root = rng() % 3037000500ULL;
        ll square = (ll)((i128)root * root);
        for (ll delta : {-1LL, 0LL, 1LL})
            if (square + delta >= 0)
                assert(integer_sqrt(square + delta) == sqrt_oracle(square + delta));
        ll a = n, b = rng() & (unsigned long long)MAX_LL;
        if (rng() & 1) a = -a;
        if (rng() & 1) b = -b;
        check_division(a, b);
        assert(binary_power(a, 1) == a);
    }
    assert(integer_sqrt(MAX_LL) == 3037000499LL);
    assert(x == 1099511627776LL); // 1LL << 40
    assert(p_int > 0 && p_uint > 0 && p_ll > 0 && p_ull > 0);
    assert(p_i128 > 0 && p_u128 > 0);
    for (long double base : {-2.0L, -0.5L, 0.5L, 1.0L, 2.0L})
        for (ll exponent = -50; exponent <= 50; ++exponent) {
            long double expected = powl(base, exponent);
            assert(fabsl(binary_power(base, exponent) / expected - 1) < 1e-12L);
        }
    assert(binary_power(-1.0L, MIN_LL) == 1.0L);
    assert(binary_power(0.0L, 0) == 1.0L);
    for (int trial = 0; trial < 5000; ++trial) {
        ll base = rng() & (unsigned long long)MAX_LL;
        if (rng() & 1) base = -base;
        ll modulus = 1 + rng() % (unsigned long long)MAX_LL;
        ll exponent = rng() % 201;
        i128 normalized = base % modulus;
        if (normalized < 0) normalized += modulus;
        i128 expected = 1 % modulus;
        for (ll i = 0; i < exponent; ++i) expected = expected * normalized % modulus;
        assert(modular_power(base, exponent, modulus) == expected);
    }
    assert(modular_power(MIN_LL, 0, 1) == 0);
    cout << "OK: power overflow regressions, shifts, 100000 random square roots "
         << "and divisions, square boundaries, floating and modular powers\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='mcduck-math-') as directory:
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
