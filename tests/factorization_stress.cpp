vector<u64> compact_factors(u64 n) {
    map<u64,int> f; factor_u64(n, f); vector<u64> out;
    for (auto [p,e] : f) while (e--) out.push_back(p);
    return out;
}

// Included after the actual LaTeX listings by factorization_stress.py.
struct FactorCase {
    string group;
    u64 n;
    vector<u64> expected;
};

vector<int> oracle_primes;

bool trial_prime32(u64 n) {
    if (n < 2) return false;
    for (u64 p : oracle_primes) {
        if (p > n / p) break;
        if (n % p == 0) return n == p;
    }
    return true;
}

u64 next_prime32(u64 n) {
    n |= 1;
    while (!trial_prime32(n)) n += 2;
    return n;
}

vector<FactorCase> make_factor_cases() {
    vector<FactorCase> cases;
    mt19937_64 gen(123456789);
    for (int i = 0; i < 100; ++i) {
        u64 p = next_prime32(0x80000000ULL + gen() % 0x7fff0000ULL);
        u64 q = i % 4 == 0 ? p : next_prime32(0x80000000ULL + gen() % 0x7fff0000ULL);
        vector<u64> f{p, q}; sort(f.begin(), f.end());
        cases.push_back({i % 4 == 0 ? "prime_square" : "balanced64", p * q, f});
        u64 small = oracle_primes[gen() % oracle_primes.size()];
        cases.push_back({"unbalanced", small * p, {small, p}});
    }
    for (int i = 0; i < 100; ++i) {
        u64 n = 1; vector<u64> f;
        for (int j = 0; j < 30; ++j) {
            u64 p = oracle_primes[gen() % 12];
            if (n <= UINT64_MAX / p) { n *= p; f.push_back(p); }
        }
        sort(f.begin(), f.end()); cases.push_back({"smooth", n, f});
    }
    for (u64 p : {2ULL, 3ULL, 5ULL, 41ULL, 97ULL, 65537ULL, 4294967291ULL}) {
        assert(trial_prime32(p));
        u64 n = p; vector<u64> f{p};
        while (n <= UINT64_MAX / p) {
            n *= p; f.push_back(p); cases.push_back({"powers", n, f});
        }
    }
    cases.push_back({"special", 1, {}});
    cases.push_back({"special", UINT64_MAX, {3, 5, 17, 257, 641, 65537, 6700417}});
    cases.push_back({"special", 3825123056546413051ULL, {149491, 747451, 34233211}});
    return cases;
}

void check_montgomery(mt19937_64& gen) {
    vector<u64> moduli{3, 5, 9, 97, 1000000007ULL, (1ULL << 63) - 1,
        (1ULL << 63) + 1, UINT64_MAX - 58, UINT64_MAX};
    for (int i = 0; i < 2000; ++i) {
        u64 n = gen() | 1ULL;
        moduli.push_back(n > 1 ? n : 3);
    }
    for (u64 n : moduli) {
        rho_optimized::Montgomery64 ctx(n);
        assert(n * ctx.inverse == 1); // Intentional unsigned wrap.
        assert(ctx.decode(ctx.one()) == 1);
        for (int i = 0; i < 100; ++i) {
            u64 a = i == 0 ? n - 1 : gen() % n;
            u64 b = i == 0 ? n - 1 : gen() % n;
            u64 x = ctx.encode(a), y = ctx.encode(b);
            assert(x < n && y < n && ctx.decode(x) == a);
            assert(ctx.decode(ctx.mul(x, y)) == (u128)a * b % n);
            assert(ctx.decode(ctx.add(x, y)) == ((u128)a + b) % n);
            u128 t = ((u128)(gen() % n) << 64) | gen();
            u64 reduced = ctx.reduce(t);
            assert(reduced < n);
            u64 r = (u128(1) << 64) % n;
            assert((u128)reduced * r % n == t % n);
        }
    }
}

void check_factor_list(u64 n, const vector<u64>& factors, const vector<u64>& expected) {
    assert(factors == expected);
    u128 product = 1;
    for (u64 p : factors) { assert(p >= 2); product *= p; }
    assert(product == n);
}

int main(int argc, char** argv) {
    factorization_test_seed = argc > 1 ? stoull(argv[1]) : 20261016;
    mt19937_64 gen(factorization_test_seed);
    // Independent Eratosthenes oracle (not the handbook's linear sieve).
    vector<bool> prime(1000001, true); prime[0] = prime[1] = false;
    for (int p = 2; p * p <= 1000000; ++p)
        if (prime[p]) for (int x = p * p; x <= 1000000; x += p) prime[x] = false;
    for (int p = 2; p <= 1000000; ++p) if (prime[p]) oracle_primes.push_back(p);
    for (u64 n = 0; n <= 1000000; ++n) {
        assert(is_prime_u64(n) == prime[n]);
        assert(rho_optimized::is_prime(n) == prime[n]);
    }
    for (u64 n = 1; n <= 100000; ++n) {
        auto trial = trial_factorize((ll)n);
        vector<u64> expected(trial.begin(), trial.end());
        check_factor_list(n, compact_factors(n), expected);
        check_factor_list(n, rho_optimized::factorize(n), expected);
    }
    check_montgomery(gen);
    for (u64 r : {0ULL, 1ULL, 2ULL, 65535ULL, 2147483648ULL, 4294967295ULL}) {
        u64 square = (u128)r * r;
        assert(rho_optimized::isqrt(square) == r);
        if (square) assert(rho_optimized::isqrt(square - 1) == r - 1);
        if (r) assert(rho_optimized::isqrt(square + 1) == r);
    }
    assert(rho_optimized::isqrt(UINT64_MAX) == 4294967295ULL);
    // Exercise the Brent kernel directly, including small-factor recovery.
    for (u64 n = 4; n <= 20000; ++n) if (!prime[n]) {
        u64 d = pollard_rho(n);
        assert(d > 1 && d < n && n % d == 0);
        d = rho_optimized::pollard(n);
        assert(d > 1 && d < n && n % d == 0);
    }
    for (int i = 0; i < 100000; ++i) {
        u64 n = gen(), r = rho_optimized::isqrt(n);
        assert((u128)r * r <= n && (u128)(r + 1) * (r + 1) > n);
    }
    assert(is_prime_u64(18446744073709551557ULL));
    assert(rho_optimized::is_prime(18446744073709551557ULL));
    for (u64 n : {561ULL, 3215031751ULL, 341550071728321ULL, 3825123056546413051ULL}) {
        assert(!is_prime_u64(n)); assert(!rho_optimized::is_prime(n));
    }
    map<u64,int> accumulated{{7, 1}};
    factor_u64(49, accumulated); assert(accumulated.at(7) == 3);
    // Official samples of GCPC 2015 F (Divisions).
    auto divisor_count = [](const vector<u64>& factors) {
        u64 count = 1;
        for (size_t i = 0; i < factors.size();) {
            size_t j = i + 1;
            while (j < factors.size() && factors[j] == factors[i]) ++j;
            count *= j - i + 1;
            i = j;
        }
        return count;
    };
    for (auto [n, expected] : vector<pair<u64,u64>>{
            {12, 6}, {999999999999999989ULL, 2}, {100000007700000049ULL, 4}}) {
        assert(divisor_count(compact_factors(n)) == expected);
        assert(divisor_count(rho_optimized::factorize(n)) == expected);
    }
    // Primality checks used by GCPC 2015 K (Upside down primes).
    for (bool montgomery : {false, true}) {
        auto test = [&](u64 n) {
            return montgomery ? rho_optimized::is_prime(n) : is_prime_u64(n);
        };
        assert(test(151));
        assert(test(18115211) && !test(11251181));
    }
    auto cases = make_factor_cases();
    for (const auto& c : cases) {
        map<u64,int> basic_map;
        factor_u64(c.n, basic_map);
        vector<u64> basic_list;
        for (auto [p, e] : basic_map)
            while (e--) basic_list.push_back(p);
        check_factor_list(c.n, basic_list, c.expected);
        check_factor_list(c.n, compact_factors(c.n), c.expected);
        check_factor_list(c.n, rho_optimized::factorize(c.n), c.expected);
    }
    cout << "OK seed=" << factorization_test_seed
         << ": primality 0..1000000, factorization 1..100000, "
         << "200900 Montgomery checks, 100000 sqrt checks, " << cases.size()
         << " known-factor cases x 2 versions\n";
    if (argc > 2 && string(argv[2]) == "1") {
        vector<pair<string, function<vector<u64>(u64)>>> implementations{
            {"compact_brent", compact_factors},
            {"optimized_montgomery_b256", rho_optimized::factorize}
        };
#ifdef HAVE_FACTORIZATION_BASELINE
        implementations.push_back({"baseline", [](u64 n) {
            map<u64,int> f; baseline::factor_u64(n, f); vector<u64> out;
            for (auto [p, e] : f) while (e--) out.push_back(p);
            return out;
        }});
#endif
        // Rotate order across seeds; timings include a full factorization.
        rotate(implementations.begin(), implementations.begin() +
               factorization_test_seed % implementations.size(), implementations.end());
        for (const auto& [name, fn] : implementations) {
            map<string, double> totals;
            for (const auto& c : cases) {
                auto start = chrono::steady_clock::now();
                auto factors = fn(c.n);
                double elapsed = chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
                check_factor_list(c.n, factors, c.expected);
                totals[c.group] += elapsed;
            }
            for (auto [group, ms] : totals) cout << "BENCH," << name << ',' << group << ',' << ms << '\n';
        }
    }
}
