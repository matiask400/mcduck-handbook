mt19937_64 rng(20261026);
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


int main(int argc, char**) {
    if(argc==1){test_mitm();cout<<"OK: 180 MITM cases vs exhaustive subsets and long long boundaries\n";return 0;}
    vector<ll> a(40); for(ll &x:a) x=2*(1+rng()%1000000000);
    ll target=accumulate(a.begin(),a.end(),0LL)/2;target|=1;
    auto start=chrono::steady_clock::now();
    assert(!subset_sum_mitm(a,target));
    cout<<fixed<<setprecision(3)<<"algorithm_ms="
        <<chrono::duration<double,milli>(chrono::steady_clock::now()-start).count()<<'\n';
}
