int main() {
    mt19937 random(123456789);

    vector<int> basic_values{4, 1, 4, 2};
    assert(sum_values(basic_values) == 11);
    normalize(basic_values);
    assert(basic_values == vector<int>({1, 2, 4}));
    assert(apply_operation(3, 5, [](int a, int b) { return a * b; }) == 15);
    assert(count_bits(0b101101ULL) == 4);
    assert(least_significant_bit_index(0b101000ULL) == 3);
    assert(most_significant_bit_index(0b101000ULL) == 5);
    assert(lowest_set_bit(0b101000ULL) == 0b1000ULL);
    ll random_value = random_integer(-7, 7);
    assert(-7 <= random_value && random_value <= 7);

    assert(first_true(0, 20, [](ll x) { return x * x >= 30; }) == 6);

    auto [negative_base, base_digits] = to_base(numeric_limits<ll>::min(), 10);
    assert(negative_base && base_digits.front() == 9 && base_digits.size() == 19);
    assert(nonempty_submasks(0b101).size() == 3);

    Compressor<ll> compressor({100, -5, 100, 7});
    assert(compressor.size() == 3 && compressor.id(7) == 1);

    PrefixSum prefix({1, 2, 3, 4});
    assert(prefix.query(1, 4) == 9);

    Fenwick fenwick(5);
    for (int i = 1; i <= 5; ++i) fenwick.add(i, i);
    assert(fenwick.query(2, 4) == 9);
    assert(fenwick.lower_bound(7) == 4);
    assert(fenwick.lower_bound(100) == 6);

    vector<ll> array{5, 1, 4, 2, 3};
    SegmentTree segment(array);
    assert(segment.query(1, 4) == 7);
    segment.set_value(2, 10);
    assert(segment.query(1, 4) == 13);

    LazySegmentTree lazy(array);
    lazy.add(1, 4, 2);
    assert(lazy.query(0, 5) == 21);
    assert(lazy.query(1, 4) == 13);

    PersistentSegmentTree persistent(array);
    int root0 = persistent.build(array);
    int root1 = persistent.set_value(root0, 2, 10);
    assert(persistent.query(root0, 0, 5) == 15);
    assert(persistent.query(root1, 0, 5) == 21);
    assert(persistent.query(root0, 1, 4) == 7); // version vieja intacta

    SparseTableMin sparse(array);
    assert(sparse.query(1, 4) == 1);
    DisjointSparseTable<ll> disjoint(array);
    assert(disjoint.query(1, 3) == 7);
    DisjointSparseTable<string> joined({"a", "b", "c", "d"});
    assert(joined.query(1, 3) == "bcd");

    DSU dsu(5);
    assert(dsu.unite(0, 1) && dsu.unite(1, 2));
    assert(dsu.same(0, 2) && dsu.size(1) == 3 && !dsu.unite(0, 2));

    assert(next_smaller({5, 2, 4, 1}) == vector<int>({1, 3, 3, 4}));
    assert(sliding_minimum({4, 2, 3, 1, 5}, 3) == vector<ll>({2, 1, 1}));

    vector<vector<pair<int,ll>>> weighted(4);
    auto add_weighted = [&](int a, int b, ll w) {
        weighted[a].push_back({b, w});
        weighted[b].push_back({a, w});
    };
    add_weighted(0, 1, 4); add_weighted(0, 2, 1);
    add_weighted(2, 1, 2); add_weighted(1, 3, 1);
    assert(dijkstra(weighted, 0)[3] == 4);
    assert(prim(weighted).value() == 4);

    vector<vector<pair<int,int>>> binary_graph(4);
    binary_graph[0] = {{1, 1}, {2, 0}};
    binary_graph[2] = {{1, 0}, {3, 1}};
    binary_graph[1] = {{3, 1}};
    assert(zero_one_bfs(binary_graph, 0) == vector<int>({0, 0, 0, 1}));

    vector<vector<ll>> all_pairs(3, vector<ll>(3, LINF));
    for (int i = 0; i < 3; ++i) all_pairs[i][i] = 0;
    all_pairs[0][1] = 4; all_pairs[1][2] = -1; all_pairs[0][2] = 10;
    floyd_warshall(all_pairs);
    assert(all_pairs[0][2] == 3);

    vector<WeightedEdge> edges{{0, 1, 1}, {1, 2, -2}, {0, 2, 5}};
    auto [bellman_distance, negative] = bellman_ford(3, edges, 0);
    assert(!negative && bellman_distance[2] == -1);

    vector<vector<int>> dag{{1, 2}, {3}, {3}, {}};
    assert(topological_sort(dag).size() == 4);
    vector<vector<int>> directed{{1}, {0, 2}, {3}, {2}};
    auto component = strongly_connected_components(directed);
    assert(component[0] == component[1] && component[2] == component[3]);
    assert(component[0] != component[2]);

    TwoSAT two_sat(2);
    two_sat.add_or(0, true, 1, true);
    two_sat.add_or(0, false, 1, true);
    two_sat.add_or(0, true, 1, false);
    auto two_sat_answer = two_sat.solve();
    assert(two_sat_answer && (*two_sat_answer)[0] && (*two_sat_answer)[1]);
    TwoSAT impossible(1);
    impossible.force(0, true); impossible.force(0, false);
    assert(!impossible.solve());

    vector<MstEdge> mst_edges{{0, 1, 4}, {0, 2, 1}, {2, 1, 2}, {1, 3, 1}};
    auto [mst_cost, chosen] = kruskal(4, mst_edges);
    assert(mst_cost == 4 && chosen.size() == 3);

    LowLink low_link(4);
    low_link.add_edge(0, 1, 0); low_link.add_edge(1, 2, 1);
    low_link.add_edge(2, 0, 2); low_link.add_edge(1, 3, 3);
    low_link.run();
    assert(low_link.bridges.size() == 1 && low_link.articulation[1]);

    vector<vector<int>> tree{{1, 2}, {0, 3, 4}, {0}, {1}, {1}};
    LCA lca(tree);
    assert(lca.query(3, 4) == 1);
    assert(lca.distance(2, 4) == 3);

    HeavyLightDecomposition hld(tree);
    vector<int> vertex_at_position(tree.size());
    for (int v = 0; v < (int)tree.size(); ++v)
        vertex_at_position[hld.position[v]] = v;
    vector<int> covered;
    hld.for_each_path(3, 2, [&](int l, int r) {
        for (int position = l; position < r; ++position)
            covered.push_back(vertex_at_position[position]);
    });
    sort(covered.begin(), covered.end());
    assert(covered == vector<int>({0, 1, 2, 3}));

    vector<vector<pair<int,int>>> directed_euler(3);
    directed_euler[0].push_back({1, 0});
    directed_euler[1].push_back({2, 1});
    directed_euler[2].push_back({0, 2});
    assert(directed_eulerian_path(directed_euler, 3).size() == 4);
    vector<vector<pair<int,int>>> undirected_euler(3);
    auto add_undirected_euler = [&](int a, int b, int id) {
        undirected_euler[a].push_back({b, id});
        undirected_euler[b].push_back({a, id});
    };
    add_undirected_euler(0, 1, 0);
    add_undirected_euler(1, 2, 1);
    assert(undirected_eulerian_path(undirected_euler, 2) ==
           vector<int>({0, 1, 2}));

    FordFulkerson ford_fulkerson(4);
    ford_fulkerson.add_edge(0, 1, 3); ford_fulkerson.add_edge(0, 2, 2);
    ford_fulkerson.add_edge(1, 2, 1); ford_fulkerson.add_edge(1, 3, 2);
    ford_fulkerson.add_edge(2, 3, 3);
    assert(ford_fulkerson.max_flow(0, 3) == 5);

    Dinic dinic(4);
    dinic.add_edge(0, 1, 3); dinic.add_edge(0, 2, 2);
    dinic.add_edge(1, 2, 1); dinic.add_edge(1, 3, 2);
    dinic.add_edge(2, 3, 3);
    assert(dinic.max_flow(0, 3) == 5);

    MinCostMaxFlow min_cost(4);
    min_cost.add_edge(0, 1, 2, 1); min_cost.add_edge(0, 2, 1, 0);
    min_cost.add_edge(1, 3, 2, 1); min_cost.add_edge(2, 3, 1, 3);
    auto min_cost_result = min_cost.run(0, 3);
    assert(min_cost_result.first == 3 && min_cost_result.second == 7);

    HopcroftKarp hopcroft_karp(3, 3);
    hopcroft_karp.add_edge(0, 0); hopcroft_karp.add_edge(0, 1);
    hopcroft_karp.add_edge(1, 1); hopcroft_karp.add_edge(2, 1);
    hopcroft_karp.add_edge(2, 2);
    assert(hopcroft_karp.maximum_matching() == 3);

    vector<vector<ll>> assignment_cost{{4, 1, 3}, {2, 0, 5}, {3, 2, 2}};
    auto [assignment_value, assignment] = hungarian(assignment_cost);
    assert(assignment_value == 5 && assignment.size() == 3);

    assert(kmp_occurrences("ababa", "aba") == vector<int>({0, 2}));
    assert(prefix_function("ababa") == vector<int>({0, 0, 1, 2, 3}));
    assert(z_function("aaaa") == vector<int>({0, 3, 2, 1}));
    RollingHash rolling("abracadabra");
    assert(rolling.get(0, 4) == rolling.get(7, 11));
    Trie trie;
    trie.insert("duck"); trie.insert("duck"); trie.insert("duke");
    assert(trie.count_word("duck") == 2 && trie.count_word("du") == 0);

    auto [odd_palindrome, even_palindrome] = manacher("abacaba");
    assert(odd_palindrome[3] == 4 &&
           *max_element(even_palindrome.begin(), even_palindrome.end()) == 0);

    AhoCorasick aho;
    aho.insert("he", 0); aho.insert("she", 1);
    aho.insert("hers", 2); aho.insert("his", 3);
    aho.build();
    auto aho_matches = aho.find_all("ahishers");
    vector<int> matched_patterns;
    for (auto [end_position, pattern_id] : aho_matches) {
        (void)end_position;
        matched_patterns.push_back(pattern_id);
    }
    sort(matched_patterns.begin(), matched_patterns.end());
    assert(matched_patterns == vector<int>({0, 1, 2, 3}));

    vector<int> banana_suffixes = suffix_array("banana");
    assert(banana_suffixes == vector<int>({5, 3, 1, 0, 4, 2}));
    assert(lcp_array("banana", banana_suffixes) == vector<int>({1, 3, 0, 0, 2}));

    assert(mod_power(2, 10, 1000000007) == 1024);
    assert(mod_inverse(3, 11).value() == 4);
    auto crt = crt_pair(2, 3, 3, 5).value();
    assert(crt.first == 8 && crt.second == 15);
    Combinations combinations(10, 1000000007);
    assert(combinations.choose(5, 2) == 10);

    Matrix fibonacci{{1, 1}, {1, 0}};
    Matrix fibonacci_power = matrix_power(fibonacci, 10, 1000000007);
    assert(fibonacci_power[0][1] == 55);
    assert(convolution({1, 2, 3}, {4, 5}) == vector<int>({4, 13, 22, 15}));

    LinearSieve sieve(100);
    assert(sieve.factorize(84) == vector<int>({2, 2, 3, 7}));
    assert(is_prime_u64(18446744073709551557ULL));
    map<u64,int> factors;
    factor_u64(8051, factors);
    assert(factors[83] == 1 && factors[97] == 1);

    vector<Point> square{{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}};
    assert(polygon_double_area(square) == 2);
    assert(convex_hull(square).size() == 4);
    assert(segments_intersect({0, 0}, {2, 2}, {0, 2}, {2, 0}));

    auto crossing = line_intersection({0, 0}, {2, 2}, {0, 2}, {2, 0});
    assert(crossing && fabsl(crossing->x - 1) < GEOMETRY_EPS &&
           fabsl(crossing->y - 1) < GEOMETRY_EPS);
    auto circle_points = circle_intersections({{0, 0}, 1}, {{1, 0}, 1});
    assert(circle_points && circle_points->size() == 2);
    vector<DPoint> double_square{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    assert(point_in_polygon(double_square, {1, 1}) == 1);
    assert(point_in_polygon(double_square, {2, 1}) == 2);
    vector<Point> integer_square{{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    assert(convex_diameter_squared(integer_square) == 2);

    assert(longest_increasing_subsequence({3, 1, 2, 5, 4}).size() == 3);
    assert(knapsack_01({2, 3, 4}, {4, 5, 7}, 5) == 9);

    vector<ll> previous(20), optimized(20, LINF), naive_layer(20, LINF);
    for (int i = 0; i < 20; ++i) previous[i] = 1LL * i * i;
    auto quadratic_cost = [](int split, int end) -> ll {
        ll difference = end - split;
        return difference * difference;
    };
    for (int end = 0; end < 20; ++end)
        for (int split = 0; split <= end; ++split)
            naive_layer[end] = min(naive_layer[end],
                                   previous[split] + quadratic_cost(split, end));
    compute_divide_conquer(0, 19, 0, 19, previous, optimized, quadratic_cost);
    assert(optimized == naive_layer);

    vector<int> mo_values{1, 2, 1, 3, 2};
    vector<MoQuery> mo_queries{{0, 3, 0}, {1, 5, 1}, {2, 4, 2}};
    std::array<int, 4> frequency{};
    ll distinct = 0;
    auto mo_add = [&](int index) {
        if (frequency[mo_values[index]]++ == 0) ++distinct;
    };
    auto mo_remove = [&](int index) {
        if (--frequency[mo_values[index]] == 0) --distinct;
    };
    assert(mos_algorithm((int)mo_values.size(), mo_queries, mo_add, mo_remove,
                         [&]() { return distinct; }) == vector<ll>({2, 3, 2}));

    // Randomized range-structure cross-checks.
    vector<ll> brute(32, 0);
    Fenwick random_fenwick(32);
    SegmentTree random_segment(brute);
    LazySegmentTree random_lazy(brute);
    for (int iteration = 0; iteration < 1000; ++iteration) {
        if (random() % 2 == 0) {
            int position = random() % 32;
            ll delta = (int)(random() % 41) - 20;
            brute[position] += delta;
            random_fenwick.add(position + 1, delta);
            random_segment.set_value(position, brute[position]);
            random_lazy.add(position, position + 1, delta);
        } else {
            int l = random() % 32, r = random() % 32;
            if (l > r) swap(l, r);
            ++r;
            ll expected = accumulate(brute.begin() + l, brute.begin() + r, 0LL);
            assert(random_fenwick.query(l + 1, r) == expected);
            assert(random_segment.query(l, r) == expected);
            assert(random_lazy.query(l, r) == expected);
        }
    }

    vector<ll> static_values(80);
    for (ll &value : static_values) value = (int)(random() % 1001) - 500;
    SparseTableMin random_sparse(static_values);
    DisjointSparseTable<ll> random_disjoint(static_values);
    for (int iteration = 0; iteration < 500; ++iteration) {
        int l = random() % 80, r = random() % 80;
        if (l > r) swap(l, r);
        ll expected_min = *min_element(static_values.begin() + l,
                                       static_values.begin() + r + 1);
        ll expected_sum = accumulate(static_values.begin() + l,
                                     static_values.begin() + r + 1, 0LL);
        assert(random_sparse.query(l, r) == expected_min);
        assert(random_disjoint.query(l, r) == expected_sum);
    }

    // Random non-negative shortest paths: Dijkstra versus Floyd-Warshall.
    const int graph_size = 12;
    vector<vector<pair<int,ll>>> random_graph(graph_size);
    vector<vector<ll>> random_dist(graph_size,
                                   vector<ll>(graph_size, LINF));
    for (int i = 0; i < graph_size; ++i) random_dist[i][i] = 0;
    for (int from = 0; from < graph_size; ++from)
        for (int to = 0; to < graph_size; ++to)
            if (from != to && random() % 4 == 0) {
                ll weight = 1 + random() % 30;
                random_graph[from].push_back({to, weight});
                random_dist[from][to] = min(random_dist[from][to], weight);
            }
    floyd_warshall(random_dist);
    for (int source = 0; source < graph_size; ++source)
        assert(dijkstra(random_graph, source) == random_dist[source]);

    // Random LCA versus a naive parent walk.
    vector<vector<int>> random_tree(100);
    vector<int> random_parent(100, 0), random_depth(100, 0);
    for (int v = 1; v < 100; ++v) {
        random_parent[v] = random() % v;
        random_depth[v] = random_depth[random_parent[v]] + 1;
        random_tree[v].push_back(random_parent[v]);
        random_tree[random_parent[v]].push_back(v);
    }
    LCA random_lca(random_tree);
    for (int iteration = 0; iteration < 500; ++iteration) {
        int a = random() % 100, b = random() % 100;
        int x = a, y = b;
        while (random_depth[x] > random_depth[y]) x = random_parent[x];
        while (random_depth[y] > random_depth[x]) y = random_parent[y];
        while (x != y) x = random_parent[x], y = random_parent[y];
        assert(random_lca.query(a, b) == x);
    }

    // KMP versus naive matching.
    for (int iteration = 0; iteration < 300; ++iteration) {
        string text(30, 'a'), pattern(1 + random() % 8, 'a');
        for (char &c : text) c += random() % 3;
        for (char &c : pattern) c += random() % 3;
        vector<int> naive;
        for (int i = 0; i + (int)pattern.size() <= (int)text.size(); ++i)
            if (text.substr(i, pattern.size()) == pattern) naive.push_back(i);
        assert(kmp_occurrences(text, pattern) == naive);
    }

    // Random 2-SAT versus exhaustive assignments.
    for (int iteration = 0; iteration < 200; ++iteration) {
        const int variables = 5;
        TwoSAT instance(variables);
        vector<tuple<int,bool,int,bool>> clauses;
        for (int clause = 0; clause < 15; ++clause) {
            int x = random() % variables, y = random() % variables;
            bool value_x = random() & 1, value_y = random() & 1;
            clauses.push_back({x, value_x, y, value_y});
            instance.add_or(x, value_x, y, value_y);
        }
        int satisfying_mask = -1;
        for (int mask = 0; mask < (1 << variables); ++mask) {
            bool valid = true;
            for (auto [x, value_x, y, value_y] : clauses)
                valid &= (((bool)(mask >> x & 1) == value_x) ||
                          ((bool)(mask >> y & 1) == value_y));
            if (valid) { satisfying_mask = mask; break; }
        }
        auto solution = instance.solve();
        assert((bool)solution == (satisfying_mask != -1));
        if (solution) {
            for (auto [x, value_x, y, value_y] : clauses)
                assert(((*solution)[x] == value_x) || ((*solution)[y] == value_y));
        }
    }

    // Random bipartite matching versus equivalent max-flow.
    for (int iteration = 0; iteration < 100; ++iteration) {
        const int left = 6, right = 6;
        HopcroftKarp matching(left, right);
        Dinic matching_flow(left + right + 2);
        int source = left + right, sink = source + 1;
        for (int v = 0; v < left; ++v) matching_flow.add_edge(source, v, 1);
        for (int v = 0; v < right; ++v) matching_flow.add_edge(left + v, sink, 1);
        for (int a = 0; a < left; ++a)
            for (int b = 0; b < right; ++b) if (random() % 3 == 0) {
                matching.add_edge(a, b);
                matching_flow.add_edge(a, left + b, 1);
            }
        assert(matching.maximum_matching() == matching_flow.max_flow(source, sink));
    }

    // Ford--Fulkerson versus Dinic on small random networks.
    for (int iteration = 0; iteration < 150; ++iteration) {
        const int vertices = 8, source = 0, sink = vertices - 1;
        FordFulkerson ford(vertices);
        Dinic dinic_reference(vertices);
        for (int from = 0; from < vertices; ++from)
            for (int to = 0; to < vertices; ++to)
                if (from != to && random() % 5 == 0) {
                    ll capacity = 1 + random() % 8;
                    ford.add_edge(from, to, capacity);
                    dinic_reference.add_edge(from, to, capacity);
                }
        assert(ford.max_flow(source, sink) ==
               dinic_reference.max_flow(source, sink));
    }

    // Random Hungarian versus brute force for 3x4 matrices.
    for (int iteration = 0; iteration < 100; ++iteration) {
        vector<vector<ll>> costs(3, vector<ll>(4));
        for (auto &row : costs)
            for (ll &value : row) value = (int)(random() % 31) - 15;
        ll brute_cost = LINF;
        for (int a = 0; a < 4; ++a)
            for (int b = 0; b < 4; ++b) if (b != a)
                for (int c = 0; c < 4; ++c) if (c != a && c != b)
                    brute_cost = min(brute_cost,
                                     costs[0][a] + costs[1][b] + costs[2][c]);
        assert(hungarian(costs).first == brute_cost);
    }

    // Random suffix arrays and NTT convolutions versus direct algorithms.
    for (int iteration = 0; iteration < 150; ++iteration) {
        string text(1 + random() % 30, 'a');
        for (char &character : text) character += random() % 4;
        vector<int> expected_suffixes(text.size());
        iota(expected_suffixes.begin(), expected_suffixes.end(), 0);
        sort(expected_suffixes.begin(), expected_suffixes.end(), [&](int a, int b) {
            return text.substr(a) < text.substr(b);
        });
        assert(suffix_array(text) == expected_suffixes);

        vector<int> first(1 + random() % 20), second(1 + random() % 20);
        for (int &value : first) value = random() % 1000;
        for (int &value : second) value = random() % 1000;
        vector<int> direct(first.size() + second.size() - 1, 0);
        for (int i = 0; i < (int)first.size(); ++i)
            for (int j = 0; j < (int)second.size(); ++j)
                direct[i + j] = (direct[i + j] + (ll)first[i] * second[j]) % NTT_MOD;
        assert(convolution(first, second) == direct);
    }

    // New handbook block: BFS, LCA O(1), rerooting and virtual tree.
    vector<vector<int>> small_tree = {{1, 2}, {0, 3, 4}, {0}, {1}, {1}};
    assert((multi_source_bfs(small_tree, {2, 4}) == vector<int>{1, 1, 0, 2, 0}));
    EulerRMQLCA rmq_lca(small_tree);
    assert(rmq_lca.lca(3, 4) == 1 && rmq_lca.lca(2, 4) == 0);
    DistanceRerooting rerooting(small_tree);
    assert((rerooting.distance_sum == vector<ll>{6, 5, 9, 8, 8}));
    vector<int> tin = {0, 1, 4, 2, 3}, tout = {5, 4, 5, 3, 4};
    auto virtual_result = build_virtual_tree(vector<int>{2, 3, 4}, tin, tout,
        [&](int a, int b) { return rmq_lca.lca(a, b); });
    assert(virtual_result.first.size() == 5 && virtual_result.second.size() == 4);

    // Lucas against direct combinations for a small prime.
    LucasTheorem lucas(7);
    assert(lucas.combination(10, 3) == 1);
    assert(lucas.combination(100, 50) == 4);

    // Offline 2D Fenwick against three point updates.
    OfflineFenwick2D fenwick_2d(5);
    for (auto [x, y] : vector<pair<int,int>>{{1, 2}, {3, 4}, {1, 5}})
        fenwick_2d.reserve_point(x, y);
    fenwick_2d.build();
    fenwick_2d.add(1, 2, 3); fenwick_2d.add(3, 4, 7); fenwick_2d.add(1, 5, 11);
    assert(fenwick_2d.rectangle_sum(0, 0, 4, 5) == 10);
    assert(fenwick_2d.rectangle_sum(1, 2, 2, 6) == 14);

    // Wavelet tree and Li Chao versus direct answers.
    vector<int> wavelet_values = {5, 1, 4, 2, 3};
    WaveletTree wavelet(wavelet_values.begin(), wavelet_values.end(), 1, 5);
    assert(wavelet.kth(1, 5, 2) == 2);
    assert(wavelet.count_leq(0, 5, 3) == 3);
    LiChaoTree li_chao(-10, 10);
    li_chao.add_line({2, 3}); li_chao.add_line({-1, 4});
    assert(li_chao.query(-2) == -1 && li_chao.query(5) == -1);

    // Gaussian elimination: unique solution x=2, y=1.
    vector<long double> linear_solution;
    assert(gaussian_elimination({{1, 1, 3}, {2, -1, 3}}, linear_solution) == 1);
    assert(fabsl(linear_solution[0] - 2) < 1e-10L);
    assert(fabsl(linear_solution[1] - 1) < 1e-10L);

    // Competition math helpers: exact square root and signed divisions.
    assert(integer_sqrt(0) == 0 && integer_sqrt(15) == 3);
    assert(integer_sqrt(16) == 4 && integer_sqrt(LLONG_MAX) == 3037000499LL);
    assert(floor_division(-7, 3) == -3 && floor_division(7, -3) == -3);
    assert(ceil_division(-7, 3) == -2 && ceil_division(7, -3) == -2);
    assert(almost_equal(0.1L + 0.2L, 0.3L));

    cout << "All smoke tests passed\n";
    return 0;
}
