// Actual before/after LaTeX listings are prepended by validate_edge_fixes.py.
mt19937_64 edge_rng(20261028);
long long checks=0;
void verify(bool ok) { ++checks; if(!ok) {cerr<<"Failed check "<<checks<<'\n';abort();} }
int random_int(int n) {return edge_rng()%n;}
void discrete_regressions() {
    after::DisjointIntervals odt;odt.intervals[0]={80,7};vector<ll> flat(80,7);
    for(int t=0;t<20000;++t) {
        int l=random_int(81),r=random_int(81);if(l>r)swap(l,r);
        if(t%3==0)r=l;
        auto snapshot=[&](){vector<tuple<int,int,ll>> v;for(auto [x,b]:odt.intervals)v.emplace_back(x,b.right,b.value);return v;};
        auto saved=snapshot();ll x=random_int(21)-10;odt.assign(l,r,x);
        if(l==r)verify(snapshot()==saved);
        fill(flat.begin()+l,flat.begin()+r,x);
        int end=0;for(auto [left,b]:odt.intervals){verify(left==end&&left<b.right);end=b.right;}
        verify(end==80);
        for(int i=0;i<80;++i){auto it=prev(odt.intervals.upper_bound(i));verify(it->first<=i&&i<it->second.right&&it->second.value==flat[i]);}
    }
    for(int lo=-15;lo<=15;++lo)for(int hi=lo;hi<=15;++hi)for(ll threshold=lo-1;threshold<=hi+2;++threshold)
        verify(after::integer_search(lo,hi,threshold)==max<ll>(lo,min<ll>(ll(hi)+1,threshold)));
    for(int t=0;t<20000;++t) {
        int lo=ll(numeric_limits<int>::min())+ll(edge_rng()%4294967296ULL);
        int hi=ll(numeric_limits<int>::min())+ll(edge_rng()%4294967296ULL);if(lo>hi)swap(lo,hi);
        ll threshold=t%3==0?ll(hi)+1:t%3==1?ll(lo)-1:ll(lo)+(edge_rng()%(ll(hi)-lo+1));
        verify(after::integer_search(lo,hi,threshold)==max<ll>(lo,min<ll>(ll(hi)+1,threshold)));
    }
    for(ll threshold:{ll(numeric_limits<int>::min())-1,0LL,ll(numeric_limits<int>::max())+1})
        verify(after::integer_search(numeric_limits<int>::min(),numeric_limits<int>::max(),threshold)==max<ll>(numeric_limits<int>::min(),min<ll>(ll(numeric_limits<int>::max())+1,threshold)));
    verify(after::integer_search(5,4,0)==5);
    for(int n=1;n<=4;++n)for(ll mod:{1LL,2LL,97LL,1000000007LL})for(int exp=0;exp<=12;++exp) {
        after::Matrix a(n,vector<ll>(n)),want(n,vector<ll>(n));
        for(auto& row:a)for(ll&x:row)x=random_int(20);
        for(int i=0;i<n;++i)want[i][i]=1%mod;
        for(int k=0;k<exp;++k){auto old=want;for(auto&row:want)fill(row.begin(),row.end(),0);for(int i=0;i<n;++i)for(int j=0;j<n;++j)for(int z=0;z<n;++z)want[i][j]=(want[i][j]+(i128)old[i][z]*a[z][j])%mod;}
        verify(after::matrix_power(a,exp,mod)==want);
    }
    for(int t=0;t<1500;++t) {
        int n=1+random_int(100);vector<ll> previous(n),current(n),want(n,LINF);
        for(ll&v:previous)v=random_int(3)==0?LINF:random_int(101)-50;
        if(t%10==0)fill(previous.begin(),previous.end(),LINF);
        auto cost=[&](int j,int i)->ll {if(t%3==0&&i%7==3)return LINF;return (ll)(i-j)*(i-j);};
        int optl=random_int(n),optr=optl+random_int(n-optl);
        after::compute_divide_conquer(0,n-1,optl,optr,previous,current,cost);
        for(int i=0;i<n;++i)for(int j=optl;j<=min(i,optr);++j)if(previous[j]!=LINF&&cost(j,i)!=LINF)want[i]=min(want[i],previous[j]+cost(j,i));
        verify(current==want);
    }
    vector<ll> previous(100000,LINF),current(100000);
    after::compute_divide_conquer(0,99999,0,99999,previous,current,[](int,int){return 0LL;});
    verify(all_of(current.begin(),current.end(),[](ll x){return x==LINF;}));
    after::compute_divide_conquer(0,99999,1,0,previous,current,[](int,int){return 0LL;});
    verify(all_of(current.begin(),current.end(),[](ll x){return x==LINF;}));
}
void check_floyd(vector<vector<ll>> d) {
    auto cost=d;int n=d.size();after::floyd_warshall(d);
    for(int s=0;s<n;++s) {
        const i128 INF128=i128(1)<<100;
        vector<i128> best(n,INF128);best[s]=0;
        for(int k=0;k<n-1;++k){auto old=best;for(int u=0;u<n;++u)if(old[u]!=INF128)for(int v=0;v<n;++v)if(cost[u][v]!=LINF)best[v]=min(best[v],old[u]+cost[u][v]);}
        vector<char> bad(n);
        for(int u=0;u<n;++u)if(best[u]!=INF128)for(int v=0;v<n;++v)if(cost[u][v]!=LINF&&best[u]+cost[u][v]<best[v])bad[v]=true;
        for(int k=0;k<n;++k)for(int u=0;u<n;++u)if(bad[u])for(int v=0;v<n;++v)if(cost[u][v]!=LINF)bad[v]=true;
        for(int v=0;v<n;++v) {
            verify(after::negative_cycle_between(d,s,v)==bool(bad[v]));
            verify(d[s][v]>=-LINF);
            if(!bad[v])verify(d[s][v]==(best[v]==INF128?i128(LINF):best[v]));
        }
    }
}
void floyd_regressions() {
    check_floyd({});check_floyd({{0}});check_floyd({{-1}});
    vector<vector<ll>> all(40,vector<ll>(40,-1));for(int i=0;i<40;++i)all[i][i]=0;check_floyd(all);
    for(auto& row:all)for(ll& value:row)value=-LINF/100;
    for(int i=0;i<40;++i)all[i][i]=0;check_floyd(all);
    for(int t=0;t<1500;++t) {
        int n=1+random_int(9);vector<vector<ll>>d(n,vector<ll>(n,LINF));for(int i=0;i<n;++i)d[i][i]=0;
        for(int u=0;u<n;++u)for(int v=0;v<n;++v)if(random_int(4)==0)d[u][v]=min(d[u][v],ll(random_int(21)-10));
        check_floyd(d);
    }
    // Near sentinel limits, but every finite simple path is representable.
    for(int sign:{-1,1}) {
        vector<vector<ll>>d(20,vector<ll>(20,LINF));for(int i=0;i<20;++i)d[i][i]=0;
        for(int i=0;i<19;++i)d[i][i+1]=sign*(LINF/25);check_floyd(d);
    }
}
int components(int n,const vector<pair<int,int>>& edges,int removed_vertex=-1,int removed_edge=-1) {
    vector<vector<int>>g(n);for(int i=0;i<(int)edges.size();++i)if(i!=removed_edge){auto [a,b]=edges[i];if(a!=removed_vertex&&b!=removed_vertex){g[a].push_back(b);g[b].push_back(a);}}
    vector<char>seen(n);int count=0;
    for(int v=0;v<n;++v)if(v!=removed_vertex&&!seen[v]){++count;vector<int>q{v};seen[v]=true;for(int at=0;at<(int)q.size();++at)for(int to:g[q[at]])if(!seen[to]){seen[to]=true;q.push_back(to);}}
    return count;
}
void tree_regressions() {
    vector<vector<int>> empty;after::DistanceRerooting rr_empty(empty);verify(rr_empty.distance_sum.empty());
    for(int t=0;t<300;++t) {
        int n=1+random_int(75),root=random_int(n);vector<vector<int>>tree(n);
        for(int i=1;i<n;++i){int p=t%3==0?i-1:t%3==1?0:random_int(i);tree[p].push_back(i);tree[i].push_back(p);}
        for(auto& adj:tree)shuffle(adj.begin(),adj.end(),edge_rng);
        after::HLD h(tree,root);before::HLD old_h(tree,root);
        verify(h.parent==old_h.parent&&h.depth==old_h.depth&&h.size==old_h.size&&h.heavy==old_h.heavy&&h.head==old_h.head&&h.pos==old_h.pos);
        after::EulerRMQLCA euler(tree,root);before::EulerRMQLCA old_euler(tree,root);
        verify(euler.euler==old_euler.euler&&euler.first==old_euler.first&&euler.depth==old_euler.depth);
        after::DistanceRerooting rr(tree);before::DistanceRerooting old_rr(tree);
        verify(rr.subtree_size==old_rr.subtree_size&&rr.distance_sum==old_rr.distance_sum);
        vector<int> inverse(n);for(int i=0;i<n;++i)inverse[h.pos[i]]=i;
        for(int s=0;s<n;++s) {
            vector<int>distance(n,-1),q{s};distance[s]=0;
            for(int at=0;at<n;++at)for(int v:tree[q[at]])if(distance[v]<0){distance[v]=distance[q[at]]+1;q.push_back(v);}
            verify(rr.distance_sum[s]==accumulate(distance.begin(),distance.end(),0LL));
            for(int v=0;v<n;++v) {
                int a=s,b=v;while(h.depth[a]>h.depth[b])a=h.parent[a];while(h.depth[b]>h.depth[a])b=h.parent[b];while(a!=b){a=h.parent[a];b=h.parent[b];}
                verify(euler.lca(s,v)==a&&euler.distance(s,v)==distance[v]);
                if(v<8) {
                    vector<int>path;for(int x=s;x!=a;x=h.parent[x])path.push_back(x);path.push_back(a);for(int x=v;x!=a;x=h.parent[x])path.push_back(x);
                    vector<int>got;h.for_each_path(s,v,[&](int l,int r){for(int p=l;p<r;++p)got.push_back(inverse[p]);});sort(got.begin(),got.end());sort(path.begin(),path.end());verify(got==path);
                }
            }
            auto [l,r]=h.subtree_interval(s);for(int p=l;p<r;++p){int v=inverse[p];while(h.depth[v]>h.depth[s])v=h.parent[v];verify(v==s);}
        }
    }
    for(int t=0;t<2000;++t) {
        int n=1+random_int(9),m=random_int(22);vector<pair<int,int>>edges;after::LowLink low(n);before::LowLink old(n);
        for(int i=0;i<m;++i){int a=random_int(n),b=random_int(n);edges.push_back({a,b});low.add_edge(a,b,i);old.add_edge(a,b,i);}
        low.run();old.run();verify(low.tin==old.tin&&low.low==old.low&&low.bridges==old.bridges&&low.articulation==old.articulation);
        int base=components(n,edges);set<pair<int,int>>want,got;
        for(int i=0;i<m;++i)if(components(n,edges,-1,i)>base){auto[a,b]=edges[i];want.insert(minmax(a,b));}
        for(auto[a,b]:low.bridges)got.insert(minmax(a,b));verify(got==want);
        for(int v=0;v<n;++v)verify(bool(low.articulation[v])==(components(n,edges,v)>base));
    }
}
template<bool Old>void benchmark(string name,string shape) {
    using H=conditional_t<Old,before::HLD,after::HLD>;
    using E=conditional_t<Old,before::EulerRMQLCA,after::EulerRMQLCA>;
    using R=conditional_t<Old,before::DistanceRerooting,after::DistanceRerooting>;
    using L=conditional_t<Old,before::LowLink,after::LowLink>;
    double ms=0;uint64_t checksum=0;
    auto timed=[&](auto fn){auto start=chrono::steady_clock::now();fn();ms+=chrono::duration<double,milli>(chrono::steady_clock::now()-start).count();};
    if(name=="hld"||name=="lowlink"||name=="reroot"||name=="euler") {
        int n=200000;vector<vector<int>>tree(n);L low(name=="lowlink"?n:0);
        for(int i=1;i<n;++i){int p=shape=="chain"?i-1:shape=="star"?0:random_int(i);tree[p].push_back(i);tree[i].push_back(p);if(name=="lowlink")low.add_edge(p,i,i-1);}
        if(name=="hld")timed([&]{H h(tree);verify(h.size[0]==n);checksum=h.size[0]+h.timer;if(shape=="chain")for(int i=0;i<n;++i)verify(h.size[i]==n-i&&h.pos[i]==i&&h.head[i]==0);});
        if(name=="lowlink")timed([&]{low.run();verify((int)low.bridges.size()==n-1);checksum=low.bridges.size();if(shape=="chain")for(int i=0;i<n;++i)verify(bool(low.articulation[i])==(i>0&&i<n-1));});
        if(name=="reroot")timed([&]{R rr(tree);checksum=rr.distance_sum[0];if(shape=="chain")for(int i=0;i<n;++i)verify(rr.distance_sum[i]==(ll)i*(i+1)/2+(ll)(n-1-i)*(n-i)/2);});
        if(name=="euler")timed([&]{E lca(tree);checksum=lca.euler.size()+lca.distance(0,n-1);if(shape=="chain")for(int i=0;i<n;++i)verify(lca.lca(i,n-1)==i&&lca.distance(0,i)==i);});
    } else if(name=="floyd") {
        int n=240;vector<vector<ll>>d(n,vector<ll>(n));for(int i=0;i<n;++i)for(int j=0;j<n;++j)d[i][j]=i==j?0:1+random_int(100000);
        timed([&]{if constexpr(Old)before::floyd_warshall(d);else after::floyd_warshall(d);});for(auto&row:d)for(ll x:row)checksum+=x;
    } else if(name=="binary") {
        timed([&]{for(int i=0;i<1000000;++i){int threshold=(ll)i*499979%1000000001;ll x=Old?before::integer_search(0,1000000000,threshold):after::integer_search(0,1000000000,threshold);checksum+=x;}});
    } else if(name=="odt") {
        using D=conditional_t<Old,before::DisjointIntervals,after::DisjointIntervals>;D d;d.intervals[0]={100000,0};vector<array<int,3>>ops(100000);
        for(auto&[l,r,x]:ops){l=random_int(100000);r=min(100000,l+1+random_int(1000));x=random_int(1000);}
        timed([&]{for(auto[l,r,x]:ops)d.assign(l,r,x);});for(auto[l,b]:d.intervals)checksum+=(uint64_t)b.value*(b.right-l);
    } else if(name=="matrix") {
        vector<vector<ll>>a(24,vector<ll>(24));for(auto&row:a)for(ll&x:row)x=random_int(100);
        timed([&]{auto result=Old?before::matrix_power(a,1234567,1000000007):after::matrix_power(a,1234567,1000000007);for(auto&row:result)for(ll x:row)checksum+=x;});
    } else if(name=="divide") {
        int n=100000;vector<ll>previous(n),current(n);for(ll&x:previous)x=random_int(1000000);
        auto cost=[](int j,int i)->ll{return (ll)(i-j)*(i-j);};
        timed([&]{if constexpr(Old)before::compute_divide_conquer(0,n-1,0,n-1,previous,current,cost);else after::compute_divide_conquer(0,n-1,0,n-1,previous,current,cost);});for(ll x:current)checksum+=x;
    } else abort();
    cout<<setprecision(12)<<"{\"ms\":"<<ms<<",\"checksum\":\""<<checksum<<"\"}\n";
}
int main(int argc,char**argv) {
    if(argc==1){discrete_regressions();floyd_regressions();tree_regressions();cout<<"OK: "<<checks<<" edge-case and differential checks\n";return 0;}
    if(string(argv[2])=="before")benchmark<true>(argv[1],argc>3?argv[3]:"random");
    else benchmark<false>(argv[1],argc>3?argv[3]:"random");
}
