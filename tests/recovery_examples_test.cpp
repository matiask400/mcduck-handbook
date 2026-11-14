int knapsack_checks = 0, hamiltonian_checks = 0, flow_checks = 0, hld_checks = 0;

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

struct InputEdge { int from, to; ll capacity; };
void check_flow(int n, const vector<InputEdge>& edges) {
    Dinic plain(n); DinicRecovered recovered(n);
    vector<pair<int,int>> ids;
    for (auto e : edges) {
        plain.add_edge(e.from, e.to, e.capacity);
        ids.push_back(recovered.add_tracked_edge(e.from, e.to, e.capacity));
    }
    ll expected = LINF;
    for (int mask = 0; mask < (1 << n); ++mask) {
        if (!(mask & 1) || (mask >> (n - 1) & 1)) continue;
        ll capacity = 0;
        for (auto e : edges) if ((mask >> e.from & 1) && !(mask >> e.to & 1))
            capacity += e.capacity;
        expected = min(expected, capacity);
    }
    ll answer = recovered.max_flow(0, n - 1);
    assert(answer == expected && answer == plain.max_flow(0, n - 1));
    auto side = recovered.min_cut_side(0);
    assert(side[0] && !side[n - 1]);
    vector<ll> balance(n, 0); ll cut_capacity = 0;
    for (int i = 0; i < (int)edges.size(); ++i) {
        auto e = edges[i]; ll flow = recovered.edge_flow(ids[i]);
        assert(0 <= flow && flow <= e.capacity);
        balance[e.from] -= flow; balance[e.to] += flow;
        if (side[e.from] && !side[e.to]) cut_capacity += e.capacity;
    }
    assert(cut_capacity == answer && balance[0] == -answer && balance[n - 1] == answer);
    for (int v = 1; v + 1 < n; ++v) assert(balance[v] == 0);
    ++flow_checks;
}

void check_hld(mt19937_64& rng, int n) {
    vector<vector<int>> tree(n);
    vector<vector<pair<int,ll>>> weighted(n);
    for (int v = 1; v < n; ++v) {
        int parent = rng() % v; ll weight = (int)(rng() % 41) - 20;
        tree[v].push_back(parent); tree[parent].push_back(v);
        weighted[v].push_back({parent, weight}); weighted[parent].push_back({v, weight});
    }
    int root = rng() % n;
    HLDEdges hld(tree, root);
    vector<ll> base(n, 0);
    vector<int> vertex_at_position(n);
    for (int v = 0; v < n; ++v) {
        vertex_at_position[hld.pos[v]] = v;
        if (v != root) for (auto [to, weight] : weighted[v])
            if (to == hld.parent[v]) base[hld.pos[v]] = weight;
    }
    SegmentTree segment(base);
    for (int query = 0; query < 100; ++query) {
        if (n > 1 && query % 4 == 0) {
            int v = rng() % n; if (v == root) v = (v + 1) % n;
            int parent = hld.parent[v]; ll value = (int)(rng() % 41) - 20;
            for (auto& edge : weighted[v]) if (edge.first == parent) edge.second = value;
            for (auto& edge : weighted[parent]) if (edge.first == v) edge.second = value;
            segment.set_value(hld.pos[v], value);
        }
        int source = rng() % n, target = query % 5 == 0 ? source : (int)(rng() % n);
        vector<int> previous(n, -1); vector<ll> distance(n, 0);
        queue<int> q; q.push(source); previous[source] = source;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (auto [to, weight] : weighted[v]) if (previous[to] == -1) {
                previous[to] = v; distance[to] = distance[v] + weight; q.push(to);
            }
        }
        ll result = 0; vector<char> included(n, false);
        hld.for_each_edge_path(source, target, [&](int left, int right) {
            assert(0 <= left && left < right && right <= n);
            for (int position = left; position < right; ++position) {
                int v = vertex_at_position[position];
                assert(v != root && !included[v]); included[v] = true;
            }
            result += segment.query(left, right);
        });
        assert(result == distance[target]);
        ++hld_checks;
    }
}

void check_output(void (*example)(), const string& expected) {
    ostringstream captured; auto* previous = cout.rdbuf(captured.rdbuf());
    example(); cout.rdbuf(previous); assert(captured.str() == expected);
}

int main() {
    check_output(knapsack_recovery_example, "9\n0 1 ");
    check_output(hamiltonian_recovery_example, "2\n0 1 2 ");
    check_output(hld_edge_example, "13\n16\n0\n");
    check_output(dinic_recovery_example,
        "5\n0->1: 3\nCorte: 0->1\n0->2: 2\nCorte: 0->2\n1->2: 1\n1->3: 2\n2->3: 3\n");
    check_knapsack({}, {}, 0);
    check_knapsack({0, 0, 1}, {5, 7, 9}, 0);
    check_knapsack({0, 1, 2}, {-5, -1, -2}, 10);
    check_knapsack({1, 1, 1}, {1, 1, 1}, 2);
    check_hamiltonian({{0}});
    check_hamiltonian(vector<vector<ll>>(4, vector<ll>(4, LINF)));
    check_flow(2, {});
    check_flow(3, {{0,1,2}, {0,1,3}, {1,0,4}, {1,2,5}, {0,2,0}});
    mt19937_64 rng(20261026);
    for (int trial = 0; trial < 500; ++trial) {
        int n = rng() % 13; vector<int> weight(n); vector<ll> value(n);
        for (int i = 0; i < n; ++i) { weight[i] = rng() % 11; value[i] = (int)(rng() % 31) - 10; }
        check_knapsack(weight, value, rng() % 31);
        n = 1 + rng() % 8;
        vector<vector<ll>> cost(n, vector<ll>(n, LINF));
        for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
            if (u != v && rng() % 3) cost[u][v] = (int)(rng() % 31) - 15;
        check_hamiltonian(cost);
        n = 2 + rng() % 6; vector<InputEdge> edges;
        for (int i = 0, count = rng() % 50; i < count; ++i) {
            int u = rng() % n, v = rng() % n;
            if (u != v) edges.push_back({u, v, (ll)(rng() % 10)});
        }
        check_flow(n, edges);
        if (trial < 200) check_hld(rng, 1 + rng() % 80);
    }
    cout << "OK: " << knapsack_checks << " knapsacks, " << hamiltonian_checks
         << " Hamiltonian paths, " << flow_checks << " flow/cut networks and "
         << hld_checks << " HLD edge queries; all four examples verified\n";
}
