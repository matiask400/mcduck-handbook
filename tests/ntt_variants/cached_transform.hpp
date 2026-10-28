// Experimental replacement for number_theoretic_transform; not installed in the book.
// Keep the book's convolution, bit reversal, root and modular-power dependency.
void number_theoretic_transform(vector<int> &values, bool inverse) {
    int n = values.size();
    assert(n > 0 && (n & (n - 1)) == 0 && n <= (1 << 23));
    static vector<int> roots(2, 1);
    for (int half = roots.size() / 2; half < n / 2; half *= 2) {
        int root = mod_power(NTT_ROOT, (NTT_MOD - 1) / (4 * half), NTT_MOD);
        roots.resize(4 * half);
        for (int i = 2 * half; i < 4 * half; ++i)
            roots[i] = (ll)roots[i / 2] * (i & 1 ? root : 1) % NTT_MOD;
    }
    if (inverse) reverse(values.begin() + 1, values.end());
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(values[i], values[j]);
    }
    for (int half = 1; half < n; half *= 2)
        for (int start = 0; start < n; start += 2 * half)
            for (int offset = 0; offset < half; ++offset) {
                int even = values[start + offset];
                int odd = (ll)roots[half + offset] * values[start + half + offset] % NTT_MOD;
                values[start + offset] = even + odd;
                if (values[start + offset] >= NTT_MOD) values[start + offset] -= NTT_MOD;
                values[start + half + offset] = even - odd;
                if (values[start + half + offset] < 0) values[start + half + offset] += NTT_MOD;
            }
    if (inverse) {
        int inverse_n = mod_power(n, NTT_MOD - 2, NTT_MOD);
        for (int &value : values) value = (ll)value * inverse_n % NTT_MOD;
    }
}
