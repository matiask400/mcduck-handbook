#include <bits/stdc++.h>
using namespace std;
using u64 = uint64_t;
using u128 = __uint128_t;

constexpr array<u64, 12> SMALL_PRIMES64 = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
};

template<class Mod>
u64 power_mod_encoded(const Mod& ctx, u64 base, u64 exponent) {
    u64 result = ctx.one();
    while (exponent) {
        if (exponent & 1) result = ctx.mul(result, base);
        base = ctx.mul(base, base);
        exponent >>= 1;
    }
    return result;
}

template<class Mod>
bool is_prime_impl_u64(u64 n) {
    if (n < 2) return false;
    for (u64 p : SMALL_PRIMES64)
        if (n % p == 0) return n == p;

    int s = 0;
    u64 d = n - 1;
    while ((d & 1) == 0) d >>= 1, ++s;
    Mod ctx(n);
    u64 one = ctx.one(), minus_one = ctx.encode(n - 1);
    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL,
                  9780504ULL, 1795265022ULL}) {
        if (a % n == 0) continue;
        u64 x = power_mod_encoded(ctx, ctx.encode(a), d);
        if (x == one || x == minus_one) continue;
        bool composite = true;
        for (int r = 1; r < s; ++r) {
            x = ctx.mul(x, x);
            if (x == minus_one) { composite = false; break; }
        }
        if (composite) return false;
    }
    return true;
}

// Requiere n compuesto. BLOCK debe ser positivo.
template<class Mod, u64 BLOCK = 256>
u64 pollard_brent_u64(u64 n) {
    static_assert(BLOCK > 0);
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;
    static mt19937_64 rng(chrono::steady_clock::now()
                          .time_since_epoch().count());
    constexpr u64 LIMIT = 1ULL << 18;
    Mod ctx(n);
    while (true) {
        u64 c = ctx.encode(uniform_int_distribution<u64>(1, n - 1)(rng));
        u64 y = ctx.encode(uniform_int_distribution<u64>(0, n - 1)(rng));
        u64 x = 0, saved = y, divisor = 1;
        u64 length = 1, batch = 0;
        auto advance = [&](u64 value) {
            return ctx.add(ctx.mul(value, value), c);
        };
        while (divisor == 1 && length <= LIMIT) {
            x = y;
            for (u64 i = 0; i < length; ++i) y = advance(y);
            for (u64 offset = 0; offset < length && divisor == 1;
                 offset += batch) {
                saved = y;
                batch = min(BLOCK, length - offset);
                u64 product = ctx.one();
                for (u64 i = 0; i < batch; ++i) {
                    y = advance(y);
                    u64 difference = x > y ? x - y : y - x;
                    product = ctx.mul(product, difference);
                }
                divisor = gcd(product, n);
            }
            length <<= 1;
        }
        if (divisor == n) {
            divisor = 1;
            for (u64 i = 0; i < batch && divisor == 1; ++i) {
                saved = advance(saved);
                u64 difference = x > saved ? x - saved : saved - x;
                divisor = gcd(difference, n);
            }
        }
        if (divisor > 1 && divisor < n) return divisor;
    }
}

// sqrtl solo aproxima; las correcciones enteras hacen exacta la raiz.
u64 isqrt_u64(u64 n) {
    u64 r = (u64)sqrtl((long double)n);
    while ((u128)r * r > n) --r;
    while ((u128)(r + 1) * (r + 1) <= n) ++r;
    return r;
}

template<class Mod, u64 BLOCK = 256>
void factor_collect_u64(u64 n, vector<u64>& out) {
    if (n == 1) return;
    u64 r = isqrt_u64(n);
    if ((u128)r * r == n) {
        size_t begin = out.size();
        factor_collect_u64<Mod, BLOCK>(r, out);
        size_t end = out.size();
        for (size_t i = begin; i < end; ++i) {
            u64 p = out[i];
            out.push_back(p);
        }
        return;
    }
    if (is_prime_impl_u64<Mod>(n)) { out.push_back(n); return; }
    u64 d = pollard_brent_u64<Mod, BLOCK>(n);
    factor_collect_u64<Mod, BLOCK>(d, out);
    factor_collect_u64<Mod, BLOCK>(n / d, out);
}

template<class Mod, u64 BLOCK = 256>
vector<u64> prime_factors_impl_u64(u64 n) {
    assert(n >= 1); // 0 no tiene factorizacion prima finita.
    vector<u64> out;
    out.reserve(64);
    // Quitar factores pequenos una sola vez, antes de la recursion.
    for (u64 p : SMALL_PRIMES64) {
        while (n % p == 0) { out.push_back(p); n /= p; }
        if (n == 1) break;
    }
    factor_collect_u64<Mod, BLOCK>(n, out);
    sort(out.begin(), out.end());
    return out;
}

struct Montgomery64 {
    u64 mod, inverse = 1, r2;
    explicit Montgomery64(u64 n) : mod(n) {
        assert(n > 1 && (n & 1));
        // Inversa positiva de n modulo 2^64; overflow unsigned intencional.
        for (int i = 0; i < 6; ++i)
            inverse *= 2 - mod * inverse;
        u64 r = (u128(1) << 64) % mod;
        r2 = (u128)r * r % mod;
    }
    // Requiere t < mod * 2^64. Todo producto de residuos < mod cumple.
    u64 reduce(u128 t) const {
        u64 q = (u64)t * inverse;
        u64 hi = t >> 64;
        u64 subtract = ((u128)q * mod) >> 64;
        return hi >= subtract ? hi - subtract : mod - (subtract - hi);
    }
    u64 encode(u64 a) const { return mul(a % mod, r2); }
    u64 decode(u64 a) const { return reduce(a); }
    u64 one() const { return reduce(r2); }
    u64 mul(u64 a, u64 b) const { return reduce((u128)a * b); }
    u64 add(u64 a, u64 b) const {
        return a >= mod - b ? a - (mod - b) : a + b;
    }
};

vector<u64> prime_factors_montgomery_u64(u64 n) {
    return prime_factors_impl_u64<Montgomery64, 256>(n);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string s;
    cin >> s;
    int take = min(10, (int)s.size());
    int zeros = (int)s.size() - take;
    u64 prefix = 0;
    for (int i = 0; i < take; ++i)
        prefix = prefix * 10 + (s[i] - '0');

    vector<u64> raw = prime_factors_montgomery_u64(prefix);
    map<u64, int> factors;
    for (u64 p : raw) ++factors[p];
    if (zeros > 0) {
        factors[2] += zeros;
        factors[5] += zeros;
    }
    cout << factors.size() << '\n';
    for (auto [p, e] : factors)
        cout << p << ' ' << e << '\n';
}
