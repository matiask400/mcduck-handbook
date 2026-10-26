// Compiled after the actual listings/headers by compare_kactl.py.
using U=uint64_t;
mt19937_64 cmp_rng(20261026);
int ri(int n){return int(cmp_rng()%n);}
U mix(U h,U v){return (h^v)*1099511628211ULL;}
template<class T>U hash_vector(const vector<T>& a){U h=1469598103934665603ULL;for(auto x:a)h=mix(h,U(x));return h;}
using Clock=chrono::steady_clock;
double elapsed(Clock::time_point start){return chrono::duration<double,milli>(Clock::now()-start).count();}
void emit(double ms,U h,string scenario){
    cout<<setprecision(12)<<"{\"algorithm_ms\":"<<ms<<",\"checksum\":\""<<h
        <<"\",\"scenario\":\""<<scenario<<"\",\"long_double_mantissa_bits\":"
        <<numeric_limits<long double>::digits<<"}\n";
}
struct Rooting{
    vi parent,depth;
    Rooting(const vector<vi>& tree):parent(tree.size(),-1),depth(tree.size()){
        vi q{0};parent[0]=0;
        for(int at=0;at<sz(q);++at)for(int v:tree[q[at]])if(parent[v]<0){parent[v]=q[at];depth[v]=depth[q[at]]+1;q.push_back(v);}
    }
};
vector<vi> random_tree(int n){vector<vi> tree(n);for(int i=1;i<n;++i){int p=ri(i);tree[p].push_back(i);tree[i].push_back(p);}return tree;}

void verify(){
    int count=0;
    for(int t=0;t<300;++t){
        int n=ri(70);string s(n,'a');for(char& c:s)c+=ri(4);
        auto a=ours::suffix_array(s);ksuffix::SuffixArray b(s);
        assert(vector<int>(b.sa.begin()+1,b.sa.end())==a);
        auto lcp=ours::lcp_array(s,a);
        for(int i=0;i<sz(lcp);++i)assert(lcp[i]==b.lcp[i+2]);
        assert(ours::z_function(s)==kz::Z(s));assert(ours::prefix_function(s)==kkmp::pi(s));
        auto [odd,even]=ours::manacher(s);auto p=kmanacher::manacher(s);
        for(int i=0;i<n;++i)assert(odd[i]==p[1][i]+1&&even[i]==p[0][i]);
        string pat(1+ri(10),'a');for(char& c:pat)c+=ri(4);
        assert(ours::kmp_occurrences(s,pat)==kkmp::match(s,pat));
        vector<ll> values(n);for(ll& x:values)x=ri(101)-50;
        auto seq=ours::longest_increasing_subsequence(values);auto ids=klis::lis(values);
        assert(seq.size()==ids.size());for(int i=1;i<sz(ids);++i)assert(ids[i-1]<ids[i]&&values[ids[i-1]]<values[ids[i]]);
        vector<int> x(ri(15)),y(ri(15));for(int& v:x)v=ri(998244353);for(int& v:y)v=ri(998244353);
        auto conv=ours::convolution(x,y);auto other=kntt::conv(vector<ll>(x.begin(),x.end()),vector<ll>(y.begin(),y.end()));
        assert(vector<ll>(conv.begin(),conv.end())==other);
        int rows=1+ri(5),columns=rows+ri(3);vector<vector<ll>> c(rows,vector<ll>(columns));for(auto& row:c)for(ll& v:row)v=ri(51)-25;
        assert(ours::hungarian(c).first==khungarian::weightedMatching(c).first);
        U number=cmp_rng()%(1ULL<<40);assert(ours::is_prime_u64(number)==kfactor::isPrime(number));
        count+=10;
    }
    // Common-contract Aho output sets; ordering and representation differ.
    for(int t=0;t<100;++t){
        vector<string> patterns;for(int j=0;j<20;++j){string s(1+ri(5),'a');for(char& c:s)c+=ri(3);patterns.push_back(s);}
        string s(100,'a');for(char& c:s)c+=ri(3);
        ours::AhoCorasick a;for(int j=0;j<sz(patterns);++j)a.insert(patterns[j],j);a.build();kaho::AhoCorasick b(patterns);
        auto actual=a.find_all(s);auto grouped=b.findAll(patterns,s);vector<pii> expected;
        for(int i=0;i<sz(grouped);++i)for(int id:grouped[i])expected.push_back({i+sz(patterns[id])-1,id});
        sort(all(actual));sort(all(expected));assert(actual==expected);++count;
    }
    for(int t=0;t<200;++t){
        int n=1+ri(8);vector<vi> graph(n);vector<vector<bool>> reach(n,vector<bool>(n));
        for(int i=0;i<n;++i){reach[i][i]=true;for(int j=0;j<n;++j)if(ri(4)==0){graph[i].push_back(j);reach[i][j]=true;}}
        for(int k=0;k<n;++k)for(int i=0;i<n;++i)for(int j=0;j<n;++j)reach[i][j]=reach[i][j]||(reach[i][k]&&reach[k][j]);
        auto a=ours::strongly_connected_components(graph);kscc::scc(graph,[](vi&){});
        for(int i=0;i<n;++i)for(int j=0;j<n;++j){bool same=reach[i][j]&&reach[j][i];assert((a[i]==a[j])==same);assert((kscc::comp[i]==kscc::comp[j])==same);++count;}
    }
    for(int trial=0;trial<50;++trial){
        int n=1+ri(6),sink=2*n+1;ours::MinCostMaxFlow a(sink+1);kmcmf::MCMF b(sink+1);
        for(int i=0;i<n;++i){a.add_edge(0,1+i,1,0);b.addEdge(0,1+i,1,0);a.add_edge(1+n+i,sink,1,0);b.addEdge(1+n+i,sink,1,0);}
        for(int i=0;i<n;++i)for(int j=0;j<n;++j){ll cost=ri(101)-50;a.add_edge(1+i,1+n+j,1,cost);b.addEdge(1+i,1+n+j,1,cost);}
        b.setpi(0);assert(a.run(0,sink)==b.maxflow(0,sink));++count;
    }
    // Cases whose contracts differ: preserve evidence instead of asserting
    // identical behavior outside KACTL's narrower contract.
    auto empty_ours=ours::kmp_occurrences("abc","");auto empty_kth=kkmp::match("abc","");
    vector<vector<ll>> negative(40,vector<ll>(40,-1));for(int i=0;i<40;++i)negative[i][i]=0;
    kfloyd::floydWarshall(negative);assert(negative[0][39]==-kfloyd::inf);
    const U big_prime=754221686909783899ULL;
    bool simple=ours::is_prime_u64(big_prime),optimized=ours::rho_optimized::is_prime(big_prime);
    assert(simple&&optimized);
    cout<<"{\"small_case_checks\":"<<count<<",\"kmp_empty_ours_count\":"<<empty_ours.size()
        <<",\"kmp_empty_kactl_count\":"<<empty_kth.size()<<",\"kactl_floyd_negative_cycle\":true,"
        <<"\"large_prime\":{\"n\":\""<<big_prime<<"\",\"ours_simple\":"<<simple
        <<",\"ours_optimized\":"<<optimized<<",\"kactl\":"<<kfactor::isPrime(big_prime)<<"},"
        <<"\"long_double_mantissa_bits\":"<<numeric_limits<long double>::digits<<"}\n";
}

void run(string name,bool kth){
    int n=200000,q=1000000;
    if(name=="fenwick"||name=="fenwick_lower_bound"){
        struct Op{int p,l,r;ll value;};vector<Op> ops(q);
        for(auto& o:ops){o.p=ri(n);o.l=ri(n+1);o.r=ri(n+1);if(o.l>o.r)swap(o.l,o.r);o.value=ri(100);}
        auto start=Clock::now();U h=0;
        if(kth){kbit::FT bit(n);for(auto o:ops){bit.update(o.p,o.value);h=mix(h,name=="fenwick"?bit.query(o.r)-bit.query(o.l):bit.lower_bound(o.value*100000LL+1)+1);}}
        else{ours::Fenwick bit(n);for(auto o:ops){bit.add(o.p+1,o.value);h=mix(h,name=="fenwick"?bit.query(o.l+1,o.r):bit.lower_bound(o.value*100000LL+1));}}
        emit(elapsed(start),h,"n=200000; 1000000 update+query pairs; ll sums; nonnegative values");
    }else if(name=="segment_sum"){
        struct Op{int p,l,r;ll v;};vector<Op> ops(q);for(auto& o:ops){o.p=ri(n);o.l=ri(n+1);o.r=ri(n+1);if(o.l>o.r)swap(o.l,o.r);o.v=ri(1000000);}
        auto start=Clock::now();U h=0;
        if(kth){ksegment::Tree st(n);for(auto o:ops){st.update(o.p,o.v);h=mix(h,st.query(o.l,o.r));}}
        else{ours::SegmentTree st(n);for(auto o:ops){st.set_value(o.p,o.v);h=mix(h,st.query(o.l,o.r));}}
        emit(elapsed(start),h,"n=200000; 1000000 point assignments+sum queries; both ll");
    }else if(name=="sparse_rmq"){
        vector<ll> a(n);for(ll& x:a)x=ri(1000000);vector<pii> queries(q);for(auto& [l,r]:queries){l=ri(n);r=ri(n);if(l>r)swap(l,r);++r;}
        auto start=Clock::now();U h=0;
        if(kth){krmq::RMQ<ll> rmq(a);for(auto [l,r]:queries)h=mix(h,rmq.query(l,r));}
        else{ours::SparseTableMin rmq(a);for(auto [l,r]:queries)h=mix(h,rmq.query(l,r-1));}
        emit(elapsed(start),h,"n=200000; preprocessing+1000000 nonempty min queries; ll");
    }else if(name=="dsu"){
        vector<pii> edges(q);for(auto& [a,b]:edges)a=ri(n),b=ri(n);auto start=Clock::now();U h=0;
        if(kth){kdsu::UF d(n);for(auto [a,b]:edges){h=mix(h,d.join(a,b));h=mix(h,d.sameSet(a,b));}}
        else{ours::DSU d(n);for(auto [a,b]:edges){h=mix(h,d.unite(a,b));h=mix(h,d.same(a,b));}}
        emit(elapsed(start),h,"n=200000; 1000000 union+connectivity pairs");
    }else if(name=="scc"||name=="scc_cycles"||name=="topological"){
        vector<vi> g(n);
        if(name=="scc_cycles"){
            for(int i=0;i<n;++i){int base=i/100*100;g[i].push_back(base+(i+1)%100);g[i].push_back(base+ri(100));}
        }else for(int i=1;i<n;++i){g[i].push_back(ri(i));g[i].push_back(ri(i));}
        auto start=Clock::now();U h=0;
        double ms;
        if(name=="scc"||name=="scc_cycles"){
            vi components;
            if(kth){kscc::scc(g,[](vi&){});ms=elapsed(start);components=kscc::comp;}
            else{components=ours::strongly_connected_components(g);ms=elapsed(start);}
            // Compare the complete partition, independently of component IDs.
            vector<int> smallest(n,n);
            for(int i=0;i<n;++i)smallest[components[i]]=min(smallest[components[i]],i);
            for(int i=0;i<n;++i)h=mix(h,smallest[components[i]]);
        }else{
            vi order=kth?ktopo::topoSort(g):ours::topological_sort(g);ms=elapsed(start);h=hash_vector(order);
        }
        emit(ms,h,name=="scc_cycles"?"V=200000 E=400000; 2000 cyclic SCCs of size 100":"DAG V=200000 E=399998; full result");
    }else if(name=="lca_binary"||name=="lca_rmq"){
        auto tree=random_tree(n);vector<pii> queries(q);for(auto& [a,b]:queries)a=ri(n),b=ri(n);
        auto start=Clock::now();U h=0;
        if(name=="lca_binary"){
            if(kth){Rooting root(tree);auto table=klift::treeJump(root.parent);for(auto [a,b]:queries)h=mix(h,klift::lca(table,root.depth,a,b));}
            else{ours::LCA lca(tree);for(auto [a,b]:queries)h=mix(h,lca.query(a,b));}
        }else{
            if(kth){klca::LCA lca(tree);for(auto [a,b]:queries)h=mix(h,lca.lca(a,b));}
            else{ours::EulerRMQLCA lca(tree);for(auto [a,b]:queries)h=mix(h,lca.lca(a,b));}
        }
        emit(elapsed(start),h,"random tree n=200000; full preprocessing+1000000 LCA queries; root 0");
    }else if(name=="dinic"){
        int width=120,layers=60;n=width*layers+2;int sink=n-1;vector<tuple<int,int,ll>> es;
        for(int j=0;j<width;++j){es.emplace_back(0,1+j,1000);es.emplace_back(1+(layers-1)*width+j,sink,1000);}
        for(int layer=0;layer<layers-1;++layer)for(int j=0;j<width;++j)for(int k=0;k<5;++k)
            es.emplace_back(1+layer*width+j,1+(layer+1)*width+ri(width),1+ri(1000));
        auto start=Clock::now();U h;
        if(kth){kflow::Dinic flow(n);for(auto [a,b,c]:es)flow.addEdge(a,b,c);h=flow.calc(0,sink);}
        else{ours::Dinic flow(n);for(auto [a,b,c]:es)flow.add_edge(a,b,c);h=flow.max_flow(0,sink);}
        emit(elapsed(start),h,"layered network V=7202 E=35640; capacities 1..1000; build+maxflow");
    }else if(name=="hopcroft_karp"||name=="vertex_cover"){
        n=30000;vector<vi> g(n);for(auto& edges:g)for(int j=0;j<6;++j)edges.push_back(ri(n));
        auto start=Clock::now();U h;
        if(kth){
            if(name=="vertex_cover"){auto cover=kcover::cover(g,n,n);h=cover.size();}
            else{vi right(n,-1);h=kmatch::hopcroftKarp(g,right);}
        }else{
            ours::HopcroftKarp match(n,n);for(int i=0;i<n;++i)for(int v:g[i])match.add_edge(i,v);
            h=match.maximum_matching();if(name=="vertex_cover"){auto [l,r]=ours::minimum_vertex_cover(match);h=l.size()+r.size();}
        }
        emit(elapsed(start),h,"bipartite 30000+30000 vertices E=180000; includes maximum matching");
    }else if(name=="min_cost_flow"){
        int width=800,sink=2*width+1;n=sink+1;vector<tuple<int,int,ll,ll>> edges;
        for(int i=0;i<width;++i){edges.emplace_back(0,1+i,1,0);edges.emplace_back(1+width+i,sink,1,0);}
        for(int i=0;i<width;++i)for(int j=0;j<10;++j)edges.emplace_back(1+i,1+width+ri(width),1,ri(2001)-1000);
        auto start=Clock::now();pair<ll,ll> answer;
        if(kth){kmcmf::MCMF flow(n);for(auto [u,v,c,w]:edges)flow.addEdge(u,v,c,w);flow.setpi(0);answer=flow.maxflow(0,sink);}
        else{ours::MinCostMaxFlow flow(n);for(auto [u,v,c,w]:edges)flow.add_edge(u,v,c,w);answer=flow.run(0,sink);}
        emit(elapsed(start),mix(U(answer.first),U(answer.second)),"weighted bipartite flow V=1602 E=9600; unit capacities; negative costs; includes potentials initialization");
    }else if(name=="hungarian"){
        n=600;vector<vector<ll>> c(n,vector<ll>(n));for(auto& row:c)for(ll& x:row)x=ri(1000000)-500000;
        auto start=Clock::now();ll result=kth?khungarian::weightedMatching(c).first:ours::hungarian(c).first;
        emit(elapsed(start),U(result),"complete 600x600 cost matrix; negative and positive ll costs");
    }else if(name=="suffix_random"||name=="suffix_repeated"){
        n=500000;string s(n,'a');if(name=="suffix_random")for(char& c:s)c+=ri(4);
        vector<int> sa,lcp;auto start=Clock::now();double ms;
        if(kth){ksuffix::SuffixArray suffix(s);ms=elapsed(start);sa.assign(suffix.sa.begin()+1,suffix.sa.end());lcp.assign(suffix.lcp.begin()+2,suffix.lcp.end());}
        else{sa=ours::suffix_array(s);lcp=ours::lcp_array(s,sa);ms=elapsed(start);}
        emit(ms,mix(hash_vector(sa),hash_vector(lcp)),"500000 characters; suffix array+LCP; sentinel removed outside timing");
    }else if(name=="manacher"||name=="z"||name=="kmp"){
        n=3000000;string s(n,'a');for(char& c:s)c+=ri(4);auto start=Clock::now();double ms;U h=0;
        if(name=="z"){auto a=kth?kz::Z(s):ours::z_function(s);ms=elapsed(start);h=hash_vector(a);}
        else if(name=="kmp"){
            string pat=s.substr(30,500);start=Clock::now();auto result=kth?kkmp::match(s,pat):ours::kmp_occurrences(s,pat);ms=elapsed(start);h=hash_vector(result);
        }else{
            if(kth){auto p=kmanacher::manacher(s);ms=elapsed(start);for(int i=0;i<n;++i){h=mix(h,p[1][i]+1);h=mix(h,p[0][i]);}}
            else{auto [odd,even]=ours::manacher(s);ms=elapsed(start);for(int i=0;i<n;++i){h=mix(h,odd[i]);h=mix(h,even[i]);}}
        }
        emit(ms,h,"text length 3000000 alphabet=4; KMP pattern length 500");
    }else if(name=="aho_random"||name=="aho_suffixes"){
        vector<string> patterns;string text;
        if(name=="aho_random"){
            for(int i=0;i<20000;++i){string s(20,'a');for(char& c:s)c+=ri(6);patterns.push_back(s);}
            text=string(1000000,'a');for(char& c:text)c+=ri(6);
        }else{for(int i=1;i<=300;++i)patterns.push_back(string(i,'a'));text=string(20000,'a');}
        auto start=Clock::now();U h=0;double ms;
        if(kth){kaho::AhoCorasick ac(patterns);auto result=ac.findAll(patterns,text);ms=elapsed(start);
            for(int i=0;i<sz(result);++i)for(int id:result[i])h+=mix(U(i+patterns[id].size()-1),U(id));}
        else{ours::AhoCorasick ac;for(int i=0;i<sz(patterns);++i)ac.insert(patterns[i],i);ac.build();auto result=ac.find_all(text);ms=elapsed(start);
            for(auto [end,id]:result)h+=mix(U(end),U(id));}
        emit(ms,h,name=="aho_random"?"20000 patterns length 20; text 1000000; build+enumerate":"patterns a^1..a^300; text a^20000; build+all matches; output layouts differ");
    }else if(name=="ntt"||name=="ntt_boundary"){
        int na=1<<18,nb=na;if(name=="ntt_boundary")na=1<<17,nb=na+1;
        vector<int>a(na),b(nb);for(int& x:a)x=ri(998244353);for(int& x:b)x=ri(998244353);
        // Convert types outside timing: KACTL stores coefficients in ll.
        vector<ll> aa(a.begin(),a.end()),bb(b.begin(),b.end());auto start=Clock::now();double ms;U h;
        if(kth){auto result=kntt::conv(aa,bb);ms=elapsed(start);h=hash_vector(result);}
        else{auto result=ours::convolution(a,b);ms=elapsed(start);h=hash_vector(result);}
        emit(ms,h,name=="ntt"?"convolution 262144+262144 coefficients; modulo 998244353":"convolution 131072+131073; result length exactly 262144");
    }else if(name=="lis"){
        n=1000000;vector<ll> a(n);for(ll& x:a)x=ri(1000000);auto start=Clock::now();U h;
        if(kth){auto ids=klis::lis(a);h=ids.size();}else{auto seq=ours::longest_increasing_subsequence(a);h=seq.size();}
        emit(elapsed(start),h,"1000000 ll values; reconstruct one strict LIS; output values versus indices");
    }else if(name=="convex_hull"){
        n=500000;vector<ours::Point> a;vector<khull::P> b;for(int i=0;i<n;++i){ll x=ri(2000001)-1000000,y=ri(2000001)-1000000;a.push_back({x,y});b.emplace_back(x,y);}
        auto start=Clock::now();double ms;vector<pair<ll,ll>> result;
        if(kth){auto hull=khull::convexHull(b);ms=elapsed(start);for(auto p:hull)result.push_back({p.x,p.y});}
        else{auto hull=ours::convex_hull(a);ms=elapsed(start);for(auto p:hull)result.push_back({p.x,p.y});}
        sort(all(result));U h=0;for(auto [x,y]:result)h=mix(mix(h,x),y);emit(ms,h,"500000 integer points; coordinates within +/-1000000; ours cross i128; KACTL cross ll");
    }else if(name=="calipers"){
        n=50000;vector<ours::Point> a;vector<khull::P> b;
        // Strictly convex integral parabola followed by its closing chord.
        for(int i=0;i<n;++i){a.push_back({i,1LL*i*i});b.emplace_back(i,1LL*i*i);}
        auto start=Clock::now();U h;
        if(kth){auto p=khull::hullDiameter(b);i128 dx=(i128)p[0].x-p[1].x,dy=(i128)p[0].y-p[1].y;h=U(dx*dx+dy*dy);}
        else h=U(ours::convex_diameter_squared(a));
        emit(elapsed(start),h,"50000 convex vertices CCW; squared distances fit signed ll");
    }else if(name=="line_queries"){
        n=100000;vi order(n);iota(all(order),0);shuffle(all(order),cmp_rng);vi queries(n);for(int& x:queries)x=ri(n);
        auto start=Clock::now();U h=0;
        if(kth){kline::LineContainer lines;for(int i:order)lines.add(2LL*i,-1LL*i*i);for(int x:queries)h=mix(h,U(-lines.query(x)));}
        else{ours::LiChaoTree lines(0,n);for(int i:order)lines.add_line({-2LL*i,1LL*i*i});for(int x:queries)h=mix(h,U(lines.query(x)));}
        emit(elapsed(start),h,"100000 active lines in random order +100000 integer min queries; max negated for KACTL");
    }else if(name=="miller_rabin"||name=="miller_rabin_simple"){
        n=10000;vector<U> values(n);for(U& x:values)x=1+cmp_rng()%1000000000000ULL;
        auto start=Clock::now();U h=0;
        for(U x:values)h=mix(h,kth?kfactor::isPrime(x):(name=="miller_rabin"?ours::rho_optimized::is_prime(x):ours::is_prime_u64(x)));
        emit(elapsed(start),h,"10000 primality queries <=10^12; common supported range");
    }else if(name=="factorization"||name=="factorization_simple"){
        vector<U> primes;for(U x=1000001;primes.size()<100;++x)if(ours::is_prime_u64(x))primes.push_back(x);
        vector<U> values(500);for(U& x:values)x=primes[ri(100)]*primes[ri(100)];
        auto start=Clock::now();U h=0;
        for(U x:values){vector<U> factors;
            if(kth){auto f=kfactor::factor(x);factors.assign(f.begin(),f.end());sort(all(factors));}
            else if(name=="factorization")factors=ours::rho_optimized::factorize(x);
            else{map<U,int> f;ours::factor_u64(x,f);for(auto [p,count]:f)for(int j=0;j<count;++j)factors.push_back(p);}
            h=mix(h,hash_vector(factors));
        }
        emit(elapsed(start),h,"500 semiprimes around 10^12; sorted prime factors; common supported range");
    }else if(name=="floyd"){
        n=350;vector<vector<ll>> a(n,vector<ll>(n));for(int i=0;i<n;++i)for(int j=0;j<n;++j)a[i][j]=i==j?0:ri(10000)+1;
        auto start=Clock::now();if(kth)kfloyd::floydWarshall(a);else ours::floyd_warshall(a);double ms=elapsed(start);
        U h=0;for(auto row:a)h=mix(h,hash_vector(row));emit(ms,h,"dense positive 350x350; KACTL also supports negative-cycle propagation");
    }else if(name=="bellman_ford"){
        n=1500;vector<ours::WeightedEdge> a;vector<kbellman::Ed>b;
        for(int i=0;i<12000;++i){int u=ri(n-1),v=u+1+ri(n-u-1),w=ri(201)-100;a.push_back({u,v,w});b.push_back({u,v,w});}
        // Include a direct edge to every vertex, so no INF encoding adapter is needed.
        for(int v=1;v<n;++v){a.push_back({0,v,1000});b.push_back({0,v,1000});}
        auto start=Clock::now();double ms;vector<ll> d;
        if(kth){vector<kbellman::Node> nodes(n);kbellman::bellmanFord(nodes,b,0);ms=elapsed(start);for(auto x:nodes)d.push_back(x.dist);}
        else{auto result=ours::bellman_ford(n,a,0);ms=elapsed(start);assert(!result.second);d=move(result.first);}
        emit(ms,hash_vector(d),"DAG V=1500 E=13499; negative weights; every vertex reachable");
    }else{cerr<<"Unknown case "<<name<<'\n';exit(2);}
}
int main(int argc,char** argv){
    if(argc!=3)return 2;
    if(string(argv[1])=="verify")verify();else run(argv[1],string(argv[2])=="kactl");
}
