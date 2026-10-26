// Compiled with the actual LaTeX listings by validate_complexity_examples.py.
mt19937 test_random(20261025);

void test_binary_search() {
    for (int n = 0; n <= 100; ++n)
        for (int threshold = -1; threshold <= n + 1; ++threshold)
            assert(first_true_example(0, n - 1, threshold) == clamp(threshold, 0, n));
    for (int i = 0; i < 1000; ++i) {
        double threshold = 1.0 + (test_random() % 1000) / 1000.0;
        int calls = 0;
        double answer = real_search(0.0, 2.0, threshold, calls);
        assert(abs(answer - sqrt(threshold)) <= 1e-6 && calls == 21);
        assert(answer * answer >= threshold);
    }
    int calls = 0;
    double left = 1e18, right = nextafter(left, numeric_limits<double>::infinity());
    assert(real_search(left, right, right * right, calls) == right && calls == 0);
    calls = 0;
    // Already within tolerance: no predicate evaluation is necessary.
    assert(real_search(1.0, 1.0 + 1e-7, 1.0 + 1e-7, calls) == 1.0 + 1e-7);
    assert(calls == 0);
}

int brute_window(const vector<ll> &a, ll limit) {
    int answer = 0;
    for (int l = 0; l < (int)a.size(); ++l) {
        ll sum = 0;
        for (int r = l; r < (int)a.size(); ++r) {
            sum += a[r];
            if (sum <= limit) answer = max(answer, r - l + 1);
        }
    }
    return answer;
}

void test_two_pointers() {
    for (int trial = 0; trial < 5000; ++trial) {
        vector<ll> a(test_random() % 40);
        for (ll &x : a) x = (int)(test_random() % 21) - 10;
        ll limit = (int)(test_random() % 61) - 30;
        assert(longest_window_signed(a, limit) == brute_window(a, limit));
        for (ll &x : a) x = abs(x);
        assert(longest_window_nonnegative(a, limit) == brute_window(a, limit));
        sort(a.begin(), a.end());
        bool found = false;
        for (int i = 0; i < (int)a.size(); ++i)
            for (int j = i + 1; j < (int)a.size(); ++j)
                found |= a[i] + a[j] == limit;
        assert(has_pair_sum(a, limit) == found);
    }
}

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

template<class Tree, class Operation>
void test_segment_variant(Operation operation, ll neutral) {
    vector<ll> values(37);
    for (ll &x : values) x = test_random() % 1000;
    Tree tree(values);
    Tree empty(0);
    assert(empty.query(0, 0) == neutral);
    for (int trial = 0; trial < 4000; ++trial) {
        if (test_random() % 2) {
            int position = test_random() % values.size();
            values[position] = test_random() % 1000;
            tree.set_value(position, values[position]);
        } else {
            int l = test_random() % (values.size() + 1);
            int r = test_random() % (values.size() + 1);
            if (l > r) swap(l, r);
            ll expected = neutral;
            for (int i = l; i < r; ++i) expected = operation(expected, values[i]);
            assert(tree.query(l, r) == expected);
        }
    }
}

int component_count(const vector<vector<pair<int,int>>> &g, int removed) {
    vector<bool> seen(g.size());
    int count = 0;
    for (int v = 0; v < (int)g.size(); ++v) if (v != removed && !seen[v]) {
        ++count;
        vector<int> pending{v};
        seen[v] = true;
        while (!pending.empty()) {
            int u = pending.back(); pending.pop_back();
            for (auto [to, id] : g[u]) if (to != removed && !seen[to]) {
                (void)id;
                seen[to] = true; pending.push_back(to);
            }
        }
    }
    return count;
}

void test_biconnected() {
    for (int trial = 0; trial < 300; ++trial) {
        int n = 1 + test_random() % 12, id = 0;
        vector<vector<pair<int,int>>> g(n);
        for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
            if (test_random() % 4 == 0) {
                int copies = 1 + (test_random() % 5 == 0);
                while (copies--) {
                    g[u].push_back({v, id}); g[v].push_back({u, id++});
                }
            }
        VertexBiconnectedComponents bcc(g);
        vector<int> appearances(n);
        for (const auto &block : bcc.components) {
            assert(set<int>(block.begin(), block.end()).size() == block.size());
            for (int v : block) ++appearances[v];
        }
        int original_count = component_count(g, -1);
        for (int v = 0; v < n; ++v) {
            assert(appearances[v] >= 1);
            assert((appearances[v] > 1) == (component_count(g, v) > original_count));
        }
    }
}

void test_virtual_tree() {
    for (int trial = 0; trial < 500; ++trial) {
        int n = 1 + test_random() % 80, timer = 0;
        vector<vector<int>> children(n);
        vector<int> parent(n), depth(n), tin(n), tout(n);
        for (int v = 1; v < n; ++v) {
            parent[v] = test_random() % v;
            children[parent[v]].push_back(v);
            depth[v] = depth[parent[v]] + 1;
        }
        function<void(int)> dfs = [&](int v) {
            tin[v] = timer++;
            for (int to : children[v]) dfs(to);
            tout[v] = timer;
        };
        dfs(0);
        auto lca = [&](int a, int b) {
            while (depth[a] > depth[b]) a = parent[a];
            while (depth[b] > depth[a]) b = parent[b];
            while (a != b) a = parent[a], b = parent[b];
            return a;
        };
        vector<int> marked(test_random() % 20);
        for (int &v : marked) v = test_random() % n;
        int calls = 0;
        auto [vertices, edges] = build_virtual_tree(marked, tin, tout, [&](int a, int b) {
            ++calls; return lca(a, b);
        });
        set<int> wanted(marked.begin(), marked.end());
        for (int a : marked) for (int b : marked) wanted.insert(lca(a, b));
        assert(set<int>(vertices.begin(), vertices.end()) == wanted);
        assert(calls == max(0, (int)set<int>(marked.begin(), marked.end()).size() - 1));
        set<pair<int,int>> wanted_edges;
        for (int v : wanted) {
            int closest = -1;
            for (int u : wanted)
                if (u != v && tin[u] <= tin[v] && tout[v] <= tout[u] &&
                    (closest == -1 || depth[u] > depth[closest])) closest = u;
            if (closest != -1) wanted_edges.insert({closest, v});
        }
        assert((set<pair<int,int>>(edges.begin(), edges.end()) == wanted_edges));
    }
}

#ifdef HAVE_BASELINE
volatile ll benchmark_checksum = 0;

template<class Function>
double median_ms(Function work) {
    vector<double> samples;
    for (int repeat = 0; repeat < 5; ++repeat) {
        auto start = chrono::steady_clock::now();
        work();
        samples.push_back(chrono::duration<double, milli>(chrono::steady_clock::now() - start).count());
    }
    sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

template<class Automaton>
size_t build_aho(const vector<string> &patterns) {
    Automaton aho;
    for (int id = 0; id < (int)patterns.size(); ++id) aho.insert(patterns[id], id);
    aho.build();
    size_t outputs = 0;
    for (const auto &node : aho.nodes) outputs += node.output.size();
    benchmark_checksum += outputs;
    return outputs;
}

void benchmark() {
    cout << fixed << setprecision(3);
    vector<string> patterns;
    for (int i = 1; i <= 200; ++i) patterns.emplace_back(i, 'a');
    string chain;
    for (int i = 0; i < 200; ++i) chain += 'b' + string(200, 'a');
    patterns.push_back(chain);
    cout << "Aho build+destroy median ms, old="
         << median_ms([&] { build_aho<before_aho::AhoCorasick>(patterns); })
         << ", new=" << median_ms([&] { build_aho<AhoCorasick>(patterns); }) << '\n';
    auto old_ids = build_aho<before_aho::AhoCorasick>(patterns);
    auto new_ids = build_aho<AhoCorasick>(patterns);
    cout << "Stored pattern IDs, old=" << old_ids << ", new=" << new_ids << '\n';
    before_aho::AhoCorasick old_aho;
    AhoCorasick new_aho;
    for (int id = 0; id < (int)patterns.size(); ++id) {
        old_aho.insert(patterns[id], id); new_aho.insert(patterns[id], id);
    }
    old_aho.build(); new_aho.build();
    string text(2000, 'a');
    assert(old_aho.find_all(text) == new_aho.find_all(text));
    cout << "Aho search+output median ms, old="
         << median_ms([&] { benchmark_checksum += old_aho.find_all(text).size(); })
         << ", new=" << median_ms([&] { benchmark_checksum += new_aho.find_all(text).size(); }) << '\n';

    vector<ll> values(8192, 1);
    vector<array<int,3>> operations;
    mt19937 generator(12345);
    for (int i = 0; i < 200000; ++i) {
        int a = generator() % values.size(), b = generator() % values.size();
        if (a > b) swap(a, b);
        operations.push_back({a, b + 1, (int)(generator() % 1000)});
    }
    auto segment_work = [&](auto &tree) {
        ll answer = 0;
        for (int i = 0; i < (int)operations.size(); ++i) {
            auto [l, r, x] = operations[i];
            if (i % 2) answer += tree.query(l, r);
            else tree.set_value(l, x);
        }
        benchmark_checksum += answer;
    };
    cout << "Segment Tree, 200000 operations median ms, old="
         << median_ms([&] { before_segment::SegmentTree tree(values); segment_work(tree); })
         << ", new=" << median_ms([&] { SegmentTree tree(values); segment_work(tree); }) << '\n';

    auto real_work = [&](auto search) {
        ll answer = 0;
        for (int i = 0; i < 100000; ++i) {
            int calls = 0;
            double threshold = 1.0 + (i % 1000) / 1000.0;
            answer += (ll)(1e6 * search(0.0, 2.0, threshold, calls));
        }
        benchmark_checksum += answer;
    };
    cout << "Real binary search, 100000 searches median ms, old="
         << median_ms([&] { real_work(old_real_search); })
         << ", new=" << median_ms([&] { real_work(real_search); }) << '\n';
    int old_calls = 0, new_calls = 0;
    double old_answer = old_real_search(0, 2, 2, old_calls);
    double new_answer = real_search(0, 2, 2, new_calls);
    cout << "Real binary search calls sqrt(2), old=" << old_calls << ", new=" << new_calls
         << scientific << "; absolute errors old=" << abs(old_answer - sqrt(2.0))
         << ", new=" << abs(new_answer - sqrt(2.0)) << fixed << '\n';
}
#endif

int main() {
    test_binary_search(); test_two_pointers(); test_aho();
    test_segment_variant<SegmentTree>(plus<ll>(), 0);
    test_segment_variant<SegmentTreeMin>([](ll a, ll b) { return min(a, b); }, LINF);
    test_segment_variant<SegmentTreeMax>([](ll a, ll b) { return max(a, b); }, -LINF);
    test_segment_variant<SegmentTreeGcd>([](ll a, ll b) { return gcd(a, b); }, 0);
    test_segment_variant<SegmentTreeXor>([](ll a, ll b) { return a ^ b; }, 0);
    test_biconnected(); test_virtual_tree();
    cout << "OK: binary search, 5000 windows/pairs, 400 Aho cases, 20000 Segment Tree operations, "
            "300 biconnected graphs and 500 virtual trees\n";
#ifdef HAVE_BASELINE
    benchmark();
#endif
}
