// Public adapters around KACTL's ll-valued core; preserve the book's int API.
// Core root is configured to 3 to preserve forward-transform values as well.
void number_theoretic_transform(vector<int> &values, bool inverse) {
    int n = values.size();
    assert(n > 0 && (n & (n - 1)) == 0 && n <= (1 << 23));
    vl work(values.begin(), values.end());
    if (inverse) reverse(work.begin() + 1, work.end());
    ntt(work);
    ll scale = inverse ? modpow(n, mod - 2) : 1;
    for (int i = 0; i < n; ++i) values[i] = work[i] * scale % mod;
}
vector<int> convolution(vector<int> a, vector<int> b) {
    if (a.empty() || b.empty()) return {};
    assert(a.size() + b.size() - 1 <= (1 << 23));
    vl result = conv(vl(a.begin(), a.end()), vl(b.begin(), b.end()));
    return vector<int>(result.begin(), result.end());
}
