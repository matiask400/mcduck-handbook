// The runner prepends the actual book listing and both experimental variants.
constexpr int P = 998244353;
using Conv = vector<int>(*)(vector<int>, vector<int>);
using Transform = void(*)(vector<int>&, bool);
Conv convolutions[] = {current::convolution, cached::convolution, kactl::convolution};
Transform transforms[] = {current::number_theoretic_transform,
                          cached::number_theoretic_transform,
                          kactl::number_theoretic_transform};
mt19937 rng(20261028);
int rand_coefficient() { return rng() % P; }
vector<int> naive(const vector<int>& a, const vector<int>& b) {
    if (a.empty() || b.empty()) return {};
    vector<int> c(a.size() + b.size() - 1);
    for (int i = 0; i < (int)a.size(); ++i)
        for (int j = 0; j < (int)b.size(); ++j)
            c[i+j] = (c[i+j] + (ll)a[i]*b[j]) % P;
    return c;
}
int tested_convolutions = 0, tested_transforms = 0;
void check_convolution(const vector<int>& a, const vector<int>& b) {
    auto expected = naive(a, b);
    for (auto f : convolutions) assert(f(a,b) == expected);
    ++tested_convolutions;
}
void correctness() {
    // Exhaustive coefficients {0,1,p-1} through length 3, including empties.
    vector<vector<int>> arrays;
    for (int n = 0; n <= 3; ++n) {
        int states = 1; for (int i = 0; i < n; ++i) states *= 3;
        for (int state = 0; state < states; ++state) {
            int x = state; vector<int> a(n);
            for (int& v : a) { int d=x%3; x/=3; v=d==2?P-1:d; }
            arrays.push_back(a);
        }
    }
    for (auto& a : arrays) for (auto& b : arrays) check_convolution(a,b);
    for (int t = 0; t < 1600; ++t) {
        vector<int> a(rng()%65), b(rng()%65);
        for (int& v : a) v=rand_coefficient();
        for (int& v : b) v=rand_coefficient();
        check_convolution(a,b);
    }
    // Grow/shrink cached roots repeatedly; test convolution padding at 2^k +- 1.
    for (int k : {1,9,3,14,2,7,16,4,10,1}) for (int d : {-1,0,1}) {
        int n=(1<<k)+d; vector<int> a(n,1), b(3,1);
        auto expected=naive(a,b);
        for (auto f : convolutions) assert(f(a,b)==expected);
        ++tested_convolutions;
    }
    // Compare forward transforms to the direct DFT, not only their inverse.
    for (int k=0;k<=6;++k) for (int t=0;t<20;++t) {
        int n=1<<k; vector<int> a(n), expected(n);
        for (int& v:a) v=rand_coefficient();
        ll w=current::mod_power(3,(P-1)/n,P), step=1;
        for(int i=0;i<n;++i) {
            ll multiplier=1, value=0;
            for(int x:a) { value=(value+x*multiplier)%P; multiplier=multiplier*step%P; }
            expected[i]=value;step=step*w%P;
        }
        for(auto f:transforms) {
            auto work=a; f(work,false); assert(work==expected);
            f(work,true); assert(work==a);
            // Also verify arbitrary frequency data: inverse -> forward.
            work=a; f(work,true); f(work,false); assert(work==a);
        }
        ++tested_transforms;
    }
    for(int k : {17,2,18,0,6,19,4}) {
        vector<int> a(1<<k);for(int& v:a)v=rand_coefficient();
        auto expected=a;transforms[0](expected,false);
        for(auto f:transforms) {
            auto work=a;f(work,false);assert(work==expected);f(work,true);assert(work==a);
        }
        ++tested_transforms;
    }
    cout << "OK: " << tested_convolutions << " convolutions per variant; "
         << tested_transforms << " transform cases per variant; naive convolution/DFT, "
         << "empty, singleton, zeros, p-1, power boundaries, cache grow/shrink\n";
}
void maximum_transform(int variant) {
    int n=1<<23; vector<int> a(n);a[1]=1;
    transforms[variant](a,false);
    ll w=current::mod_power(3,(P-1)/n,P),value=1;
    for(int x:a) {assert(x==value);value=value*w%P;}
    transforms[variant](a,true);
    for(int i=0;i<n;++i)assert(a[i]==(i==1));
    cout<<"OK: variant "<<variant<<" forward/inverse at 2^23; forward vs powers of primitive root\n";
}
uint64_t hash_result(const vector<int>& a) {
    uint64_t h=1469598103934665603ULL;
    for(int v:a)h=(h^uint64_t(v))*1099511628211ULL;
    return h;
}
void benchmark(int variant, string name, bool warm) {
    int na=1<<18,nb=na,count=1;
    if(name=="boundary") {na=1<<17;nb=na+1;}
    if(name=="above_boundary") {na=(1<<17)+1;nb=na;}
    if(name=="tiny_batch") {na=16;nb=17;count=10000;}
    if(name=="medium_batch") {na=1024;nb=1025;count=100;}
    if(name=="large_batch")count=5;
    if(name=="unbalanced") {na=1;nb=1<<18;}
    vector<int>a(na),b(nb);for(int&v:a)v=rand_coefficient();for(int&v:b)v=rand_coefficient();
    // Cache preparation explicitly excluded only for the warm measurement.
    if(warm) {auto out=convolutions[variant](a,b);assert(out.size()==a.size()+b.size()-1);}
    double ms=0;uint64_t hash=0;
    for(int i=0;i<count;++i) {
        a[i%na]=(a[i%na]+1)%P;
        auto start=chrono::steady_clock::now();
        auto out=convolutions[variant](a,b);
        ms+=chrono::duration<double,milli>(chrono::steady_clock::now()-start).count();
        // Output traversal is outside timing, but always consumes the result.
        hash^=hash_result(out)+i;
    }
    cout<<setprecision(12)<<"{\"ms\":"<<ms<<",\"convolutions\":"<<count
        <<",\"a_size\":"<<na<<",\"b_size\":"<<nb
        <<",\"checksum\":\""<<hash<<"\"}\n";
}
int main(int argc,char**argv) {
    if(argc==1){correctness();return 0;}
    int variant=stoi(argv[2]);
    if(string(argv[1])=="max"){maximum_transform(variant);return 0;}
    benchmark(variant,argv[1],argc>3&&string(argv[3])=="warm");
}
