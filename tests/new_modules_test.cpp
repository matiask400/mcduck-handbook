// The Python driver prepends the actual handbook listings and their dependencies.
mt19937_64 rng(20261026);

bool is_subsequence(const string &a, const string &b) {
    size_t i = 0;
    for (char c : b) if (i < a.size() && a[i] == c) ++i;
    return i == a.size();
}

int edit_oracle(const string &a, const string &b) {
    vector<int> previous(b.size() + 1), current(b.size() + 1);
    iota(previous.begin(), previous.end(), 0);
    for (int i = 1; i <= (int)a.size(); ++i) {
        current[0] = i;
        for (int j = 1; j <= (int)b.size(); ++j)
            current[j] = min({previous[j] + 1, current[j - 1] + 1,
                             previous[j - 1] + (a[i - 1] != b[j - 1])});
        swap(previous, current);
    }
    return previous.back();
}

void test_strings() {
    for (int trial = 0; trial < 300; ++trial) {
        string a(rng() % 9, 'a'), b(rng() % 9, 'a');
        for (char &c : a) c += rng() % 3;
        for (char &c : b) c += rng() % 3;
        auto common = longest_common_subsequence(a, b);
        assert(is_subsequence(common, a) && is_subsequence(common, b));
        int best = 0;
        for (int mask = 0; mask < (1 << a.size()); ++mask) {
            string candidate;
            for (int i = 0; i < (int)a.size(); ++i) if (mask >> i & 1) candidate += a[i];
            if (is_subsequence(candidate, b)) best = max(best, (int)candidate.size());
        }
        assert((int)common.size() == best);
        auto [cost, steps] = edit_distance(a, b);
        assert(cost == edit_oracle(a, b) && (int)steps.size() == cost);
        string transformed = a;
        for (auto step : steps) {
            assert(step.position >= 0 && step.position <= (int)transformed.size());
            if (step.type == 'I') transformed.insert(transformed.begin() + step.position, step.value);
            else {
                assert(step.position < (int)transformed.size());
                if (step.type == 'D') transformed.erase(step.position, 1);
                else { assert(step.type == 'R'); transformed[step.position] = step.value; }
            }
        }
        assert(transformed == b);
    }
    for (int trial = 0; trial < 100; ++trial) {
        string s(rng() % 8, 'a');
        for (char &c : s) c += rng() % 2;
        int best = s.size();
        for (int length = 0; length <= (int)s.size(); ++length)
            for (int mask = 0; mask < (1 << ((length + 1) / 2)); ++mask) {
                string palindrome(length, 'a');
                for (int i = 0; i < (length + 1) / 2; ++i)
                    palindrome[i] = palindrome[length - 1 - i] = 'a' + (mask >> i & 1);
                best = min(best, edit_oracle(s, palindrome));
            }
        auto [cost, palindrome] = nearest_palindrome(s);
        string reversed = palindrome; reverse(reversed.begin(), reversed.end());
        assert(palindrome == reversed && cost == best && edit_oracle(s, palindrome) == cost);
    }
    assert(nearest_palindrome("hello").first == 2);
}

void test_knapsacks() {
    for (int trial = 0; trial < 400; ++trial) {
        int n = rng() % 12, capacity = rng() % 35;
        vector<int> weight(n);
        vector<ll> value(n);
        for (int i = 0; i < n; ++i) weight[i] = 1 + rng() % 9, value[i] = (int)(rng() % 21) - 5;
        vector<ll> expected(capacity + 1);
        for (int c = 0; c <= capacity; ++c)
            for (int i = 0; i < n; ++i) if (weight[i] <= c)
                expected[c] = max(expected[c], expected[c - weight[i]] + value[i]);
        auto [score, items] = unbounded_knapsack(weight, value, capacity);
        ll actual = 0; int used = 0;
        for (int i : items) { assert(i >= 0 && i < n); used += weight[i]; actual += value[i]; }
        assert(used <= capacity && actual == score && score == expected[capacity]);
        // Zero weights are allowed in 0/1 exact sum; each index remains unique.
        for (int &w : weight) --w;
        int minimum = n + 1;
        for (int mask = 0; mask < (1 << n); ++mask) {
            int sum = 0;
            for (int i = 0; i < n; ++i) if (mask >> i & 1) sum += weight[i];
            if (sum == capacity) minimum = min(minimum, __builtin_popcount((unsigned)mask));
        }
        vector<int> all_costs;
        auto chosen = minimum_items_exact(weight, capacity, &all_costs);
        assert(all_costs.size() == (size_t)capacity + 1);
        assert(all_costs[capacity] == minimum);
        assert((bool)chosen == (minimum != n + 1));
        if (chosen) {
            assert((int)chosen->size() == minimum);
            assert(set<int>(chosen->begin(), chosen->end()).size() == chosen->size());
            int sum = 0; for (int i : *chosen) sum += weight[i];
            assert(sum == capacity);
        }
        auto reachable = subset_sums<128>(weight);
        vector<bool> oracle(128); oracle[0] = true;
        for (int w : weight) for (int sum = 127; sum >= w; --sum)
            oracle[sum] = oracle[sum] || oracle[sum - w];
        for (int sum = 0; sum < 128; ++sum) assert(reachable[sum] == oracle[sum]);
    }
    assert(subset_sums<8>({100}) == bitset<8>(1));
    assert(!subset_sums<8>({2})[4]);
}

void test_mitm() {
    for (int trial = 0; trial < 180; ++trial) {
        int n = rng() % 16;
        vector<ll> a(n); for (ll &x : a) x = (int)(rng() % 21) - 10;
        ll target = (int)(rng() % 61) - 30;
        bool possible = false;
        for (int mask = 0; mask < (1 << n); ++mask) {
            ll sum = 0;
            for (int i = 0; i < n; ++i) if (mask >> i & 1) sum += a[i];
            possible |= sum == target;
        }
        auto chosen = subset_sum_mitm(a, target);
        assert((bool)chosen == possible);
        if (chosen) {
            ll sum = 0; for (int i : *chosen) sum += a[i];
            assert(sum == target && set<int>(chosen->begin(), chosen->end()).size() == chosen->size());
        }
    }
    assert(!subset_sum_mitm({numeric_limits<ll>::max(), 0}, numeric_limits<ll>::min()));
    assert(subset_sum_mitm({numeric_limits<ll>::max()}, numeric_limits<ll>::max()));
}

void test_diameter() {
    assert(tree_diameter({}).second.empty());
    for (int trial = 0; trial < 400; ++trial) {
        int n = 1 + rng() % 30;
        vector<vector<pair<int,ll>>> tree(n);
        for (int v = 1; v < n; ++v) {
            int parent = rng() % v; ll weight = rng() % 21;
            tree[parent].push_back({v,weight}); tree[v].push_back({parent,weight});
        }
        ll best = 0;
        for (int source = 0; source < n; ++source) {
            vector<ll> d(n, -1); d[source] = 0;
            vector<int> pending{source};
            for (int i = 0; i < (int)pending.size(); ++i)
                for (auto [to,w] : tree[pending[i]]) if (d[to] == -1) {
                    d[to] = d[pending[i]] + w; pending.push_back(to);
                }
            best = max(best, *max_element(d.begin(), d.end()));
        }
        auto [length, path] = tree_diameter(tree);
        assert(length == best && !path.empty());
        assert(set<int>(path.begin(), path.end()).size() == path.size());
        ll sum = 0;
        for (int i = 1; i < (int)path.size(); ++i) {
            bool found = false;
            for (auto [to,w] : tree[path[i - 1]]) if (to == path[i]) sum += w, found = true;
            assert(found);
        }
        assert(sum == length);
    }
}

void test_konig() {
    for (int trial = 0; trial < 300; ++trial) {
        int n = rng() % 6, m = rng() % 6;
        HopcroftKarp matching(n, m);
        for (int u = 0; u < n; ++u) for (int v = 0; v < m; ++v)
            if (rng() % 3 == 0) matching.add_edge(u, v);
        int size = matching.maximum_matching();
        auto [left, right] = minimum_vertex_cover(matching);
        int mask = 0;
        for (int u : left) mask |= 1 << u;
        for (int v : right) mask |= 1 << (n + v);
        auto covers = [&](int selected) {
            for (int u = 0; u < n; ++u) for (int v : matching.graph[u])
                if (!(selected >> u & 1) && !(selected >> (n + v) & 1)) return false;
            return true;
        };
        assert(covers(mask) && (int)(left.size() + right.size()) == size);
        for (int candidate = 0; candidate < (1 << (n + m)); ++candidate)
            if (__builtin_popcount((unsigned)candidate) < size) assert(!covers(candidate));
    }
}

bool good_digits(ll value) {
    string s = to_string(value);
    for (int i = 1; i < (int)s.size(); ++i) if (s[i] == s[i - 1]) return false;
    return true;
}

void test_digit_dp() {
    vector<int> count(100001);
    for (int i = 0; i <= 100000; ++i) count[i] = (i ? count[i - 1] : 0) + good_digits(i);
    for (int trial = 0; trial < 1000; ++trial) {
        int upper = rng() % count.size();
        assert(count_no_equal_neighbors(upper) == count[upper]);
    }
    ll bound = 1, expected = 1, power = 1;
    for (int length = 1; length <= 18; ++length) {
        bound *= 10; power *= 9; expected += power;
        assert(count_no_equal_neighbors(bound - 1) == expected);
    }
    ll maximum = numeric_limits<ll>::max();
    assert(count_no_equal_neighbors(maximum) - count_no_equal_neighbors(maximum - 1) == good_digits(maximum));
    assert(count_no_equal_neighbors(-1) == 0 && count_no_equal_neighbors(0) == 1);
}

void test_xor() {
    for (int trial = 0; trial < 300; ++trial) {
        int n = rng() % 11;
        vector<uint64_t> a(n); XorBasis basis;
        vector<bitset<9>> vectors;
        for (auto &x : a) { x = rng() % 256; basis.insert(x); vectors.emplace_back(x | 256); }
        set<uint64_t> reachable;
        bool dependent_even = false;
        for (int mask = 0; mask < (1 << n); ++mask) {
            uint64_t value = 0;
            for (int i = 0; i < n; ++i) if (mask >> i & 1) value ^= a[i];
            reachable.insert(value);
            dependent_even |= mask != 0 && value == 0 && __builtin_popcount((unsigned)mask) % 2 == 0;
        }
        assert(basis.maximum() == *reachable.rbegin());
        assert(reachable.size() == (size_t(1) << basis.rank));
        for (int x = 0; x < 512; ++x) assert(basis.contains(x) == (reachable.count(x) != 0));
        auto chosen = xor_dependency<9, 10>(vectors);
        assert((bool)chosen == dependent_even);
        if (chosen) {
            assert(!chosen->empty() && chosen->size() % 2 == 0);
            assert(set<int>(chosen->begin(), chosen->end()).size() == chosen->size());
            bitset<9> value; for (int i : *chosen) value ^= vectors[i];
            assert(value.none());
        }
    }
    XorBasis high; high.insert(uint64_t(1) << 63); high.insert(7);
    assert(high.maximum() == (uint64_t(1) << 63 | 7));
    assert((!xor_dependency<3,3>({bitset<3>(5), bitset<3>(6), bitset<3>(7)})));
    // All four published examples: validate the output, not one arbitrary answer.
    vector<vector<string>> samples = {{"101","001","010","011","000"},
        {"01","10","11"}, {"011","000","100","011"}, {"000"}};
    vector<bool> possible = {true, false, true, false};
    for (int test = 0; test < 4; ++test) {
        ostringstream input, output;
        input << samples[test].size() << ' ' << samples[test][0].size() << '\n';
        for (const auto &s : samples[test]) input << s << '\n';
        istringstream in(input.str());
        auto *old_in = cin.rdbuf(in.rdbuf()); auto *old_out = cout.rdbuf(output.rdbuf());
        brazilian_footxor();
        cin.rdbuf(old_in); cout.rdbuf(old_out);
        string teams = output.str(); teams.pop_back();
        if (!possible[test]) { assert(teams == "*"); continue; }
        assert(teams.size() == samples[test].size());
        int count1 = 0, count2 = 0, value1 = 0, value2 = 0;
        for (int i = 0; i < (int)teams.size(); ++i) {
            int value = stoi(samples[test][i], nullptr, 2);
            if (teams[i] == '1') ++count1, value1 ^= value;
            else if (teams[i] == '2') ++count2, value2 ^= value;
            else assert(teams[i] == '0');
        }
        assert(count1 > 0 && count1 == count2 && value1 == value2);
    }
}

uint64_t benchmark_result = 0;
void benchmark(const string &name) {
    auto start = chrono::steady_clock::now();
    if (name.rfind("lcs_", 0) == 0) {
        int n = stoi(name.substr(4));
        string a(n, 'a'), b(n, 'a');
        for (int i = 0; i < n; ++i) { a[i] += rng() % 4; b[i] += rng() % 4; }
        auto result = longest_common_subsequence(a, b);
        assert(is_subsequence(result, a) && is_subsequence(result, b)); benchmark_result = result.size();
    } else if (name == "edit_3000" || name == "palindrome_3000") {
        string a(3000, 'a'), b(3000, 'a');
        for (int i = 0; i < 3000; ++i) { a[i] += rng() % 26; b[i] += rng() % 26; }
        if (name == "edit_3000") benchmark_result = edit_distance(a, b).first;
        else benchmark_result = nearest_palindrome(a).first;
    } else if (name == "unbounded") {
        vector<int> weights(1000); vector<ll> values(1000);
        for (int i = 0; i < 1000; ++i) { weights[i] = 1 + rng() % 1000; values[i] = rng() % 1000; }
        benchmark_result = unbounded_knapsack(weights, values, 100000).first;
    } else if (name == "exact") {
        auto chosen = minimum_items_exact(vector<int>(100,1000), 100000);
        assert(chosen && chosen->size() == 100); benchmark_result = chosen->size();
    } else if (name == "nine_packs_max") {
        vector<int> left, right;
        minimum_items_exact(vector<int>(100,1000), 100000, &left);
        minimum_items_exact(vector<int>(100,1000), 100000, &right);
        int best = 201;
        for (int sum = 1; sum <= 100000; ++sum)
            if (left[sum] <= 100 && right[sum] <= 100) best = min(best, left[sum] + right[sum]);
        assert(best == 2); benchmark_result = best;
    } else if (name == "bitset") {
        vector<int> weights(1000); for (int &x : weights) x = 1 + rng() % 1000;
        benchmark_result = subset_sums<1000001>(weights).count();
    } else if (name == "bitset_many") {
        auto sums = subset_sums<1000001>(vector<int>(100000,1));
        assert(sums.count() == 100001); benchmark_result = sums.count();
    } else if (name == "mitm_40") {
        vector<ll> a(40); for (ll &x : a) x = 2 * (1 + rng() % 1000000000);
        ll target = accumulate(a.begin(), a.end(), 0LL) / 2; target |= 1;
        assert(!subset_sum_mitm(a, target)); benchmark_result = target;
    } else if (name == "diameter_chain") {
        int n = 200000; vector<vector<pair<int,ll>>> tree(n);
        for (int i = 1; i < n; ++i) { tree[i - 1].push_back({i,1}); tree[i].push_back({i - 1,1}); }
        auto [length, path] = tree_diameter(tree);
        assert(length == n - 1 && (int)path.size() == n); benchmark_result = length;
    } else if (name == "konig") {
        int n = 100000, m = 50000; HopcroftKarp matching(n, m);
        for (int u = 0; u < n; ++u) {
            matching.add_edge(u, u % m);
            matching.add_edge(u, rng() % m); matching.add_edge(u, rng() % m);
        }
        assert(matching.maximum_matching() == m);
        auto [left, right] = minimum_vertex_cover(matching);
        assert(left.size() + right.size() == (size_t)m); benchmark_result = m;
    } else if (name == "digit") {
        for (int i = 0; i < 10000; ++i)
            benchmark_result ^= count_no_equal_neighbors(1000000000000000000LL - i);
    } else if (name == "xor_scalar") {
        XorBasis basis; for (int i = 0; i < 1000000; ++i) basis.insert(rng());
        assert(basis.rank == 64); benchmark_result = basis.maximum();
    } else if (name == "footxor_1500") {
        constexpr int n = 1500;
        vector<bitset<1501>> vectors(n);
        for (int i = 0; i < n; ++i) { vectors[i][i] = true; vectors[i][1500] = true; }
        for (int i = 0; i < 15000; ++i) {
            int a = rng() % n, b = rng() % n, c = rng() % n;
            if (a != b && a != c && b != c) vectors[a] ^= vectors[b] ^ vectors[c];
        }
        vectors.back() = vectors[0];
        auto chosen = xor_dependency<1501,1500>(vectors);
        assert(chosen && chosen->size() % 2 == 0);
        bitset<1501> value; for (int i : *chosen) value ^= vectors[i];
        assert(value.none()); benchmark_result = chosen->size();
    } else { cerr << "Unknown benchmark\n"; exit(2); }
    cout << fixed << setprecision(3) << "algorithm_and_input_generation_ms="
         << chrono::duration<double,milli>(chrono::steady_clock::now() - start).count()
         << " checksum=" << benchmark_result << '\n';
}

int main(int argc, char **argv) {
    if (argc == 2) { benchmark(argv[1]); return 0; }
    test_strings(); test_knapsacks(); test_mitm(); test_diameter();
    test_konig(); test_digit_dp(); test_xor();
    cout << "OK: 300 LCS/edit cases, 100 palindrome cases, 400 knapsacks/bitsets, "
            "180 MITM cases, 400 trees, 300 bipartite graphs, digit oracles and 300 XOR bases\n";
}
