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


mt19937 test_random(20261025);
void test_aho() {
    AhoCorasick example;
    vector<string> sample{"he", "she", "his", "hers"};
    for (int i = 0; i < (int)sample.size(); ++i) example.insert(sample[i], i);
    example.build();
    assert((example.find_all("ahishers") ==
            vector<pair<int,int>>{{3,2}, {5,1}, {5,0}, {7,3}}));
    for (int trial = 0; trial < 400; ++trial) {
        vector<string> patterns(test_random() % 16);
        AhoCorasick aho;
        for (int id = 0; id < (int)patterns.size(); ++id) {
            patterns[id] = string(1 + test_random() % 7, 'a');
            for (char &c : patterns[id]) c += test_random() % 3;
            aho.insert(patterns[id], id);
        }
        aho.build();
        size_t stored = 0;
        for (const auto &node : aho.nodes) stored += node.output.size();
        assert(stored == patterns.size()); // Inherited outputs must not be copied.
        string text(test_random() % 100, 'a');
        for (char &c : text) c += test_random() % 3;
        vector<pair<int,int>> expected;
        for (int id = 0; id < (int)patterns.size(); ++id)
            for (int pos = 0; pos + (int)patterns[id].size() <= (int)text.size(); ++pos)
                if (text.compare(pos, patterns[id].size(), patterns[id]) == 0)
                    expected.push_back({pos + (int)patterns[id].size() - 1, id});
        auto actual = aho.find_all(text);
        assert(actual == aho.find_all(text)); // Queries do not mutate the automaton.
        sort(actual.begin(), actual.end());
        sort(expected.begin(), expected.end());
        assert(actual == expected);
    }
}


int knapsack_checks=0, hamiltonian_checks=0;
void check_knapsack(const vector<int>& weight, const vector<ll>& value, int capacity) {
    int n = weight.size();
    ll expected = 0;
    for (int mask = 0; mask < (1 << n); ++mask) {
        int total_weight = 0;
        ll total_value = 0;
        for (int i = 0; i < n; ++i) if (mask >> i & 1) {
            total_weight += weight[i]; total_value += value[i];
        }
        if (total_weight <= capacity) expected = max(expected, total_value);
    }
    vector<int> chosen{99};
    ll answer = knapsack_with_items(weight, value, capacity, chosen);
    assert(answer == expected && answer == knapsack_01(weight, value, capacity));
    vector<char> seen(n, false);
    int total_weight = 0;
    ll total_value = 0;
    for (int i : chosen) {
        assert(0 <= i && i < n && !seen[i]); seen[i] = true;
        total_weight += weight[i]; total_value += value[i];
    }
    assert(total_weight <= capacity && total_value == answer);
    ++knapsack_checks;
}

void check_hamiltonian(const vector<vector<ll>>& cost) {
    int n = cost.size();
    vector<int> order(n); iota(order.begin(), order.end(), 0);
    ll expected = LINF;
    do {
        ll total = 0; bool valid = true;
        for (int i = 1; i < n; ++i) {
            ll edge = cost[order[i - 1]][order[i]];
            if (edge == LINF) { valid = false; break; }
            total += edge;
        }
        if (valid) expected = min(expected, total);
    } while (next_permutation(order.begin(), order.end()));
    vector<int> path{99};
    ll answer = hamiltonian_with_vertices(cost, path);
    assert(answer == expected && answer == hamiltonian_path(cost));
    if (answer == LINF) assert(path.empty());
    else {
        assert((int)path.size() == n);
        vector<char> seen(n, false); ll total = 0;
        for (int i = 0; i < n; ++i) {
            int v = path[i]; assert(0 <= v && v < n && !seen[v]); seen[v] = true;
            if (i) { assert(cost[path[i - 1]][v] != LINF); total += cost[path[i - 1]][v]; }
        }
        assert(total == answer);
    }
    ++hamiltonian_checks;
}


int main() {
    test_strings(); test_knapsacks(); test_digit_dp(); test_aho();
    assert(longest_increasing_subsequence({3,1,2,5,4}).size() == 3);
    for (int trial=0; trial<300; ++trial) {
        int n=rng()%10;
        vector<int> w(n); vector<ll> v(n);
        for(int i=0;i<n;++i){w[i]=rng()%8;v[i]=(int)(rng()%21)-10;}
        check_knapsack(w,v,rng()%20);
        n=1+rng()%7;
        vector<vector<ll>> cost(n,vector<ll>(n,LINF));
        for(int a=0;a<n;++a)for(int b=0;b<n;++b)
            if(a!=b&&rng()%3)cost[a][b]=(int)(rng()%21)-10;
        check_hamiltonian(cost);
    }
    assert(kmp_occurrences("ababa","aba")==vector<int>({0,2}));
    assert(z_function("aaaa")==vector<int>({0,3,2,1}));
    assert(suffix_array("banana")==vector<int>({5,3,1,0,4,2}));
    cout << "OK: LCS/edit/palindromes, knapsack variants, digit DP, LIS, "
            "300 optional DP recoveries, 400 Aho cases and KMP/Z/suffix arrays\n";
}
