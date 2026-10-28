// Included after unchanged listings by audit_book.py. Deterministic oracles.
mt19937 audit_rng(20261026);
int rnd(int n) { return int(audit_rng()%n); }
long long audit_checks = 0;
void check(bool ok) {
    ++audit_checks;
    if (!ok) { cerr << "Failed check " << audit_checks << '\n'; exit(1); }
}

void structures() {
    for (int trial=0;trial<120;++trial) {
        int n=1+rnd(64);
        vector<ll> a(n), lazy_a(n);
        for (ll& x:a) x=rnd(101)-50;
        lazy_a=a;
        SegmentTree seg(a); LazySegmentTree lazy(a);
        PersistentSegmentTree pst(a);
        vector<int> roots{pst.build(a)};
        vector<vector<ll>> versions{a};
        SparseTableMin sparse(a); DisjointSparseTable<ll> dst(a);
        vector<int> w(a.begin(),a.end()), original=w;
        WaveletTree wave(w.begin(),w.end(),-50,50);
        Fenwick bit(n); FenwickXor bx(n);
        vector<ll> freq(n), xr(n);
        ImplicitTreap treap;
        vector<ll> sequence;
        for(int op=0;op<180;++op) {
            int l=rnd(n+1),r=rnd(n+1); if(l>r)swap(l,r);
            check(seg.query(l,r)==accumulate(a.begin()+l,a.begin()+r,0LL));
            if(l<r) {
                check(sparse.query(l,r-1)==*min_element(original.begin()+l,original.begin()+r));
                check(dst.query(l,r-1)==accumulate(original.begin()+l,original.begin()+r,0LL));
                auto s=vector<int>(original.begin()+l,original.begin()+r);sort(s.begin(),s.end());
                int k=1+rnd(r-l),v=rnd(105)-52;
                check(wave.kth(l,r,k)==s[k-1]);
                check(wave.count_leq(l,r,v)==upper_bound(s.begin(),s.end(),v)-s.begin());
            }
            int p=rnd(n);ll x=rnd(101)-50;
            seg.set_value(p,x); a[p]=x;
            lazy.add(l,r,x);for(int i=l;i<r;++i)lazy_a[i]+=x;
            check(lazy.query(l,r)==accumulate(lazy_a.begin()+l,lazy_a.begin()+r,0LL));
            int old=rnd(roots.size());auto next=versions[old];next[p]=x;
            roots.push_back(pst.set_value(roots[old],p,x));versions.push_back(next);
            int v=rnd(roots.size());
            check(pst.query(roots[v],l,r)==accumulate(versions[v].begin()+l,versions[v].begin()+r,0LL));
            int inc=rnd(7);freq[p]+=inc;bit.add(p+1,inc);
            xr[p]^=x;bx.add(p+1,x);ll want_xor=0;
            for(int i=l;i<r;++i)want_xor^=xr[i];
            check(bx.query(l+1,r)==want_xor);
            check(bit.query(l+1,r)==accumulate(freq.begin()+l,freq.begin()+r,0LL));
            ll target=rnd(int(accumulate(freq.begin(),freq.end(),0LL))+3);int pos=1;ll sum=0;
            while(pos<=n && sum+freq[pos-1]<target)sum+=freq[pos++-1];
            if(target<=0)pos=1;
            check(bit.lower_bound(target)==pos);
            if(sequence.empty()||rnd(3)==0) {
                int at=rnd(sequence.size()+1);treap.insert(at,x);sequence.insert(sequence.begin()+at,x);
            } else if(rnd(3)==0) {
                int at=rnd(sequence.size());treap.erase(at);sequence.erase(sequence.begin()+at);
            } else {
                int tl=rnd(sequence.size()+1),tr=rnd(sequence.size()+1);if(tl>tr)swap(tl,tr);
                treap.reverse_range(tl,tr);reverse(sequence.begin()+tl,sequence.begin()+tr);
            }
            check(treap.range_sum(0,sequence.size())==accumulate(sequence.begin(),sequence.end(),0LL));
            if(!sequence.empty()) {int at=rnd(sequence.size());check(treap.range_sum(at,at+1)==sequence[at]);}
        }
        while(!sequence.empty()) {treap.erase(0);sequence.pop_back();}
        DisjointIntervals odt;odt.intervals[0]={n,0};vector<ll> flat(n);
        for(int i=0;i<100;++i) {
            int l=rnd(n),r=l+1+rnd(n-l);ll x=rnd(21)-10;
            odt.assign(l,r,x);fill(flat.begin()+l,flat.begin()+r,x);
            for(int j=0;j<n;++j) {
                auto it=prev(odt.intervals.upper_bound(j));
                check(it->first<=j&&j<it->second.right&&it->second.value==flat[j]);
            }
        }
    }
    vector<int> extremes{numeric_limits<int>::min(),0,numeric_limits<int>::max()};
    WaveletTree extreme_wave(extremes.begin(),extremes.end(),numeric_limits<int>::min(),numeric_limits<int>::max());
    check(extreme_wave.kth(0,3,1)==numeric_limits<int>::min());
    SegmentTree empty(0);LazySegmentTree empty_lazy(vector<ll>{});
    check(empty.query(0,0)==0&&empty_lazy.query(0,0)==0);
    DisjointSparseTable<string> words({"a","b","c","d","e"});
    check(words.query(1,4)=="bcde");
    OfflineFenwick2D plane(17);map<pair<int,int>,ll> points;
    for(int x=0;x<17;++x)for(int y=-7;y<=7;++y)plane.reserve_point(x,y);
    plane.build();
    for(int i=0;i<2000;++i) {
        int x=rnd(17),y=rnd(15)-7,d=rnd(11)-5;plane.add(x,y,d);points[{x,y}]+=d;
        int l=rnd(18),r=rnd(18),b=rnd(19)-9,t=rnd(19)-9;
        if(l>r)swap(l,r);if(b>t)swap(b,t);ll want=0;
        for(auto [p,v]:points)if(l<=p.first&&p.first<r&&b<=p.second&&p.second<t)want+=v;
        check(plane.rectangle_sum(l,b,r,t)==want);
    }
    LiChaoTree lines(-100,100);vector<LiChaoTree::Line> all;
    for(int i=0;i<1000;++i) {
        LiChaoTree::Line line{rnd(1001)-500,rnd(1001)-500};all.push_back(line);lines.add_line(line);
        ll x=rnd(201)-100,want=numeric_limits<ll>::max();
        for(auto f:all)want=min(want,f.slope*x+f.intercept);
        check(lines.query(x)==want);
    }
}

int component_count(int n,const vector<pair<int,int>>& edges,int skip_vertex=-1,int skip_edge=-1) {
    DSU d(n);for(int i=0;i<(int)edges.size();++i) {
        auto [a,b]=edges[i];if(i!=skip_edge&&a!=skip_vertex&&b!=skip_vertex)d.unite(a,b);
    }
    set<int> roots;for(int i=0;i<n;++i)if(i!=skip_vertex)roots.insert(d.find(i));
    return roots.size();
}

bool has_euler(int n,const vector<pair<int,int>>& edges,bool directed) {
    if(edges.empty())return true;
    function<bool(int,int)> rec=[&](int v,int used) {
        if(used==(1<<(int)edges.size())-1)return true;
        for(int i=0;i<(int)edges.size();++i)if(!(used>>i&1)) {
            auto [a,b]=edges[i];
            if(a==v&&rec(b,used|(1<<i)))return true;
            if(!directed&&b==v&&rec(a,used|(1<<i)))return true;
        }
        return false;
    };
    for(int v=0;v<n;++v)if(rec(v,0))return true;
    return false;
}
void verify_euler(const vector<int>& path,const vector<pair<int,int>>& edges,bool directed) {
    check(path.size()==edges.size()+1);vector<bool> used(edges.size());
    for(int i=1;i<(int)path.size();++i) {
        bool found=false;
        for(int j=0;j<(int)edges.size();++j)if(!used[j]) {
            auto [a,b]=edges[j];
            if((a==path[i-1]&&b==path[i])||(!directed&&b==path[i-1]&&a==path[i])) {
                used[j]=true;found=true;break;
            }
        }
        check(found);
    }
}

void graphs() {
    for(int trial=0;trial<600;++trial) {
        int n=1+rnd(7),m=rnd(10);vector<pair<int,int>> edges;
        vector<vector<int>> adj(n);LowLink low(n);
        vector<vector<pair<int,int>>> und(n),dir(n);
        for(int i=0;i<m;++i) {
            int a=rnd(n),b=rnd(n);edges.push_back({a,b});adj[a].push_back(b);
            low.add_edge(a,b,i);und[a].push_back({b,i});und[b].push_back({a,i});dir[a].push_back({b,i});
        }
        int before=component_count(n,edges);low.run();
        int bridges=0;
        for(int i=0;i<m;++i)bridges+=component_count(n,edges,-1,i)>before;
        check((int)low.bridges.size()==bridges);
        for(int v=0;v<n;++v)check(bool(low.articulation[v])==(component_count(n,edges,v)>before));
        auto scc=strongly_connected_components(adj);
        vector<vector<bool>> reach(n,vector<bool>(n));
        for(int i=0;i<n;++i)reach[i][i]=true;
        for(auto [a,b]:edges)reach[a][b]=true;
        for(int k=0;k<n;++k)for(int i=0;i<n;++i)for(int j=0;j<n;++j)reach[i][j]=reach[i][j]||(reach[i][k]&&reach[k][j]);
        for(int i=0;i<n;++i)for(int j=0;j<n;++j)check((scc[i]==scc[j])==(reach[i][j]&&reach[j][i]));
        bool cycle=false;for(auto [a,b]:edges)cycle|=reach[b][a];
        auto topo=topological_sort(adj);check(topo.empty()==cycle);
        if(!cycle) {
            vector<int> position(n);for(int i=0;i<n;++i)position[topo[i]]=i;
            for(auto [a,b]:edges)check(position[a]<position[b]);
        }
        vector<vector<pair<int,ll>>> weighted(n);
        vector<vector<ll>> distances(n,vector<ll>(n,LINF));
        for(int v=0;v<n;++v)distances[v][v]=0;
        for(auto [a,b]:edges) {ll w=rnd(11);weighted[a].push_back({b,w});distances[a][b]=min(distances[a][b],w);}
        for(int k=0;k<n;++k)for(int i=0;i<n;++i)for(int j=0;j<n;++j)
            if(distances[i][k]!=LINF&&distances[k][j]!=LINF)distances[i][j]=min(distances[i][j],distances[i][k]+distances[k][j]);
        for(int s=0;s<n;++s) {
            vector<int> parent;auto recovered=dijkstra_recovered(weighted,s,parent);
            check(recovered==distances[s]&&dijkstra(weighted,s)==recovered);
            for(int t=0;t<n;++t)if(recovered[t]!=LINF) {
                int v=t,steps=0;ll cost=0;
                while(v!=s) {
                    check(++steps<n&&parent[v]>=0);
                    ll edge=LINF;for(auto [to,w]:weighted[parent[v]])if(to==v)edge=min(edge,w);
                    check(edge!=LINF);cost+=edge;v=parent[v];
                }
                check(cost==recovered[t]);
            }
        }
        for(bool directed:{false,true}) {
            auto path=directed?directed_eulerian_path(dir,m):undirected_eulerian_path(und,m);
            check(!path.empty()==has_euler(n,edges,directed));
            if(!path.empty())verify_euler(path,edges,directed);
        }
        // Brute all vertex subsets; maximal connected subsets without a cut.
        // Self-loops are excluded here and covered by the dedicated finding.
        vector<vector<pair<int,int>>> no_loops(n);int id=0;
        for(auto [a,b]:edges)if(a!=b) {no_loops[a].push_back({b,id});no_loops[b].push_back({a,id++});}
        vector<int> valid;
        for(int mask=1;mask<(1<<n);++mask) {
            bool good=true;
            for(int remove=-1;remove<n;++remove)if(remove==-1||(mask>>remove&1)) {
                int remaining=mask;if(remove>=0)remaining&=~(1<<remove);
                if(!remaining)continue;
                int visited=0;vector<int> pending{__builtin_ctz((unsigned)remaining)};
                while(!pending.empty()) {int v=pending.back();pending.pop_back();if(visited>>v&1)continue;
                    visited|=1<<v;for(auto [to,e]:no_loops[v])if(remaining>>to&1)pending.push_back(to);
                }
                if(visited!=remaining)good=false;
            }
            if(good)valid.push_back(mask);
        }
        set<int> maximal;
        for(int mask:valid) {
            bool contained=false;for(int other:valid)if(mask!=other&&(mask&other)==mask)contained=true;
            if(!contained)maximal.insert(mask);
        }
        VertexBiconnectedComponents bcc(no_loops);set<int> actual;
        for(auto block:bcc.components) {int mask=0;for(int v:block)mask|=1<<v;actual.insert(mask);}
        check(actual==maximal);
    }
    for(int trial=0;trial<700;++trial) {
        int n=1+rnd(7);TwoSAT sat(n);vector<tuple<int,bool,int,bool>> clauses;
        for(int i=0,m=rnd(24);i<m;++i) {
            int a=rnd(n),b=rnd(n);bool av=rnd(2),bv=rnd(2);
            sat.add_or(a,av,b,bv);clauses.emplace_back(a,av,b,bv);
        }
        auto valid=[&](int mask) {for(auto [a,av,b,bv]:clauses)if(bool(mask>>a&1)!=av&&bool(mask>>b&1)!=bv)return false;return true;};
        bool possible=false;for(int mask=0;mask<(1<<n);++mask)possible|=valid(mask);
        auto answer=sat.solve();check(bool(answer)==possible);
        if(answer) {int mask=0;for(int i=0;i<n;++i)if((*answer)[i])mask|=1<<i;check(valid(mask));}
    }
    for(int trial=0;trial<400;++trial) {
        int n=2+rnd(5);Dinic dinic(n);FordFulkerson ford(n);
        vector<tuple<int,int,int>> es;
        for(int i=0,m=rnd(18);i<m;++i) {
            int a=rnd(n),b=rnd(n),c=rnd(5);es.emplace_back(a,b,c);
            dinic.add_edge(a,b,c);ford.add_edge(a,b,c);
        }
        ll cut=LINF;
        for(int mask=0;mask<(1<<n);++mask)if((mask&1)&&!(mask>>(n-1)&1)) {
            ll c=0;for(auto [a,b,w]:es)if((mask>>a&1)&&!(mask>>b&1))c+=w;cut=min(cut,c);
        }
        check(dinic.max_flow(0,n-1)==cut);check(ford.max_flow(0,n-1)==cut);
    }
    for(int trial=0;trial<180;++trial) {
        int n=2+rnd(4),m=rnd(8),limit=rnd(6);MinCostMaxFlow flow(n);
        vector<tuple<int,int,int,int>> es;
        for(int i=0;i<m;++i) {int a=rnd(n-1),b=a+1+rnd(n-a-1),c=rnd(3),w=rnd(11)-5;
            es.emplace_back(a,b,c,w);flow.add_edge(a,b,c,w);}
        pair<ll,ll> best={0,0};vector<int> balance(n);
        function<void(int,ll)> enumerate=[&](int i,ll cost) {
            if(i==m) {
                if(balance[0]>0||-balance[0]>limit||balance[n-1]!=-balance[0])return;
                for(int v=1;v<n-1;++v)if(balance[v])return;
                ll sent=-balance[0];if(sent>best.first||(sent==best.first&&cost<best.second))best={sent,cost};return;
            }
            auto [a,b,c,w]=es[i];
            for(int x=0;x<=c;++x) {balance[a]-=x;balance[b]+=x;enumerate(i+1,cost+x*w);balance[a]+=x;balance[b]-=x;}
        };
        enumerate(0,0);check(flow.run(0,n-1,limit)==best);
    }
    for(int trial=0;trial<250;++trial) {
        int n=1+rnd(5),m=n+rnd(3);vector<vector<ll>> c(n,vector<ll>(m));
        for(auto& row:c)for(ll& x:row)x=rnd(41)-20;
        ll best=LINF;function<void(int,int,ll)> rec=[&](int i,int used,ll cost) {
            if(i==n) {best=min(best,cost);return;}
            for(int j=0;j<m;++j)if(!(used>>j&1))rec(i+1,used|(1<<j),cost+c[i][j]);
        };rec(0,0,0);
        auto [value,assignment]=hungarian(c);check(value==best);
        set<int> seen;ll sum=0;for(int i=0;i<n;++i) {check(seen.insert(assignment[i]).second);sum+=c[i][assignment[i]];}check(sum==value);
    }
    for(int trial=0;trial<300;++trial) {
        int n=1+rnd(6),m=rnd(10);vector<MstEdge> es;vector<vector<pair<int,ll>>> gph(n);vector<pair<int,int>> conn;
        for(int i=0;i<m;++i) {int a=rnd(n),b=rnd(n),w=rnd(21)-10;es.push_back({a,b,w});conn.push_back({a,b});gph[a].push_back({b,w});gph[b].push_back({a,w});}
        int components=component_count(n,conn);ll best=LINF;
        for(int mask=0;mask<(1<<m);++mask)if(__builtin_popcount((unsigned)mask)==n-components) {
            DSU d(n);ll cost=0;bool valid=true;
            for(int j=0;j<m;++j)if(mask>>j&1) {valid&=d.unite(es[j].u,es[j].v);cost+=es[j].weight;}
            if(valid)best=min(best,cost);
        }
        auto [value,chosen]=kruskal(n,es);check(value==best);check((int)chosen.size()==n-components);
        auto p=prim(gph);check(bool(p)==(components==1));if(p)check(*p==best);
    }
}

void trees() {
    for(int trial=0;trial<180;++trial) {
        int n=1+rnd(60),root=rnd(n);vector<vector<int>> tree(n);
        for(int i=1;i<n;++i) {int p=rnd(i);tree[p].push_back(i);tree[i].push_back(p);}
        vector<int> parent(n,-1),depth(n),tin(n),tout(n),order{root};parent[root]=root;
        for(int at=0;at<n;++at)for(int v:tree[order[at]])if(parent[v]<0) {parent[v]=order[at];depth[v]=depth[order[at]]+1;order.push_back(v);}
        auto slow_lca=[&](int a,int b) {while(depth[a]>depth[b])a=parent[a];while(depth[b]>depth[a])b=parent[b];while(a!=b)a=parent[a],b=parent[b];return a;};
        LCA lift(tree,root);EulerRMQLCA euler(tree,root);HLD hld(tree,root);DistanceRerooting rr(tree);CentroidDecomposition cd(tree);
        N=n;for(int i=0;i<(1<<K);++i)g[i].clear();for(int i=0;i<n;++i)g[i]=tree[i];lca_init();
        LCA from_zero(tree);
        vector<int> at_pos(n);for(int i=0;i<n;++i)at_pos[hld.pos[i]]=i;
        int timer=0;function<void(int,int)> dfs=[&](int v,int p) {tin[v]=timer++;for(int to:tree[v])if(to!=p)dfs(to,v);tout[v]=timer;};dfs(root,-1);
        auto ancestor=[&](int a,int b) {return tin[a]<=tin[b]&&tout[b]<=tout[a];};
        for(int v=0;v<n;++v) {
            ll sum=0;
            for(int u=0;u<n;++u) {
                int c=slow_lca(v,u);check(lift.query(v,u)==c&&euler.lca(v,u)==c);
                check(lca(v,u)==from_zero.query(v,u));sum+=depth[v]+depth[u]-2*depth[c];
                check(lift.distance(v,u)==depth[v]+depth[u]-2*depth[c]);
            }
            check(rr.distance_sum[v]==sum);
            for(int k=0;k<=depth[v];++k) {int p=v;for(int j=0;j<k;++j)p=parent[p];check(lift.jump(v,k)==p);}
            auto [l,r]=hld.subtree_interval(v);set<int> actual,expected;
            for(int p=l;p<r;++p)actual.insert(at_pos[p]);
            for(int u=0;u<n;++u)if(ancestor(v,u))expected.insert(u);
            check(actual==expected);
        }
        // Each centroid splits its own remaining component into <= half.
        for(int c=0;c<n;++c) {
            vector<char> inside(n);int count=0;
            for(int v=0;v<n;++v)for(int p=v;p!=-1;p=cd.parent[p])if(p==c) {inside[v]=true;++count;break;}
            vector<char> seen(n);seen[c]=true;
            for(int v:tree[c])if(inside[v]) {
                int size=0;vector<int> stack{v};
                while(!stack.empty()) {int u=stack.back();stack.pop_back();if(seen[u])continue;seen[u]=true;++size;for(int to:tree[u])if(inside[to]&&!seen[to])stack.push_back(to);}
                check(size<=count/2);
            }
        }
        for(int rep=0;rep<30;++rep) {
            int a=rnd(n),b=rnd(n),c=slow_lca(a,b);multiset<int> expected,actual;
            for(int v=a;v!=c;v=parent[v])expected.insert(v);
            for(int v=b;v!=c;v=parent[v])expected.insert(v);expected.insert(c);
            hld.for_each_path(a,b,[&](int l,int r){for(int p=l;p<r;++p)actual.insert(at_pos[p]);});check(actual==expected);
            vector<int> marked;for(int j=0,k=rnd(20);j<k;++j)marked.push_back(rnd(n));
            auto [vertices,edges]=build_virtual_tree(marked,tin,tout,[&](int x,int y){return lift.query(x,y);});
            if(marked.empty()) {check(vertices.empty()&&edges.empty());continue;}
            set<int> closure(marked.begin(),marked.end());
            for(int x:marked)for(int y:marked)closure.insert(slow_lca(x,y));
            check(set<int>(vertices.begin(),vertices.end())==closure);check(edges.size()+1==vertices.size());
            for(auto [p,v]:edges) {check(p!=v&&ancestor(p,v));for(int x:vertices)if(x!=v&&ancestor(x,v))check(depth[x]<=depth[p]);}
        }
    }
}

void numeric() {
    for(int trial=0;trial<1000;++trial) {
        ll a=(((u64)audit_rng()<<32)|audit_rng())>>1;
        ll b=(((u64)audit_rng()<<32)|audit_rng())>>1,m=1+rnd(1000000000);
        if(rnd(2))a=-a;if(rnd(2))b=-b;
        auto norm=[&](i128 x) {x%=m;return (ll)(x<0?x+m:x);};
        check(mod_add(a,b,m)==norm((i128)a+b));check(mod_subtract(a,b,m)==norm((i128)a-b));check(mod_multiply(a,b,m)==norm((i128)a*b));
        ll x,y,g=extended_gcd(abs(a),abs(b),x,y);check((i128)abs(a)*x+(i128)abs(b)*y==g);
        auto inv=mod_inverse(a,m);check(bool(inv)==(gcd(mod_normalize(a,m),m)==1));if(inv)check(mod_multiply(a,*inv,m)==1%m);
        int n=1+rnd(40),k=1+rnd(40);vector<int> av(n),bv(k),expected(n+k-1);
        for(int& v:av)v=rnd(NTT_MOD);for(int& v:bv)v=rnd(NTT_MOD);
        for(int i=0;i<n;++i)for(int j=0;j<k;++j)expected[i+j]=(expected[i+j]+(ll)av[i]*bv[j])%NTT_MOD;
        check(convolution(av,bv)==expected);
    }
    check(convolution({}, {1}).empty());
    mt19937_64 wide_rng(20261026);
    for(int trial=0;trial<10000;++trial) {
        u64 mod=wide_rng()|1ULL;if(mod==1)mod=3;
        rho_optimized::Montgomery64 ctx(mod);
        u64 a=wide_rng()%mod,b=wide_rng()%mod;
        check(ctx.decode(ctx.encode(a))==a);
        check(ctx.decode(ctx.mul(ctx.encode(a),ctx.encode(b)))==(u128)a*b%mod);
        check(ctx.decode(ctx.add(ctx.encode(a),ctx.encode(b)))==((u128)a+b)%mod);
        if(trial<1000)check(is_prime_u64(mod)==rho_optimized::is_prime(mod));
    }
    for(int p:{2,3,5,7,11,31}) {
        LucasTheorem lucas(p);vector<ll> row{1};
        for(int n=0;n<160;++n) {
            for(int k=0;k<=n;++k)check(lucas.combination(n,k)==row[k]);
            vector<ll> next(n+2,1);for(int k=1;k<=n;++k)next[k]=(row[k-1]+row[k])%p;row=next;
        }
    }
    for(int n=0;n<20000;++n) {
        bool prime=n>=2;for(int d=2;d<=n/d;++d)if(n%d==0)prime=false;
        check(is_prime_u64(n)==prime);check(rho_optimized::is_prime(n)==prime);
    }
    for(u64 n:{1ULL,4ULL,561ULL,41041ULL,3215031751ULL,1000000016000000063ULL,18446744073709551615ULL,18446744030759878681ULL}) {
        auto fs=rho_optimized::factorize(n);u128 product=1;map<u64,int> simple;factor_u64(n,simple);map<u64,int> grouped;
        for(u64 f:fs) {check(is_prime_u64(f));product*=f;++grouped[f];}check(product==n);check(grouped==simple);
    }
    for(int trial=0;trial<300;++trial) {
        int n=1+rnd(5);Matrix a(n,vector<ll>(n));for(auto& row:a)for(ll& x:row)x=rnd(10);
        int exponent=rnd(12);ll mod=2+rnd(1000000000);Matrix expected(n,vector<ll>(n));for(int i=0;i<n;++i)expected[i][i]=1;
        for(int step=0;step<exponent;++step) {Matrix next(n,vector<ll>(n));for(int i=0;i<n;++i)for(int j=0;j<n;++j)for(int k=0;k<n;++k)next[i][j]=(next[i][j]+(i128)expected[i][k]*a[k][j])%mod;expected=next;}
        check(matrix_power(a,exponent,mod)==expected);
        vector<vector<long double>> mat(n,vector<long double>(n+1));vector<long double> target(n);
        for(auto& x:target)x=rnd(21)-10;
        for(int i=0;i<n;++i) {for(int j=0;j<n;++j)mat[i][j]=(i==j?100:0)+rnd(11)-5;for(int j=0;j<n;++j)mat[i][n]+=mat[i][j]*target[j];}
        vector<long double> solution;check(gaussian_elimination(mat,solution)==1);for(int i=0;i<n;++i)check(fabsl(solution[i]-target[i])<1e-8L);
        vector<int> permutation(n);iota(permutation.begin(),permutation.end(),0);long double det=0;
        do {int inversions=0;long double term=1;for(int i=0;i<n;++i) {term*=mat[i][permutation[i]];for(int j=i+1;j<n;++j)inversions+=permutation[i]>permutation[j];}det+=(inversions&1)?-term:term;}while(next_permutation(permutation.begin(),permutation.end()));
        for(auto& row:mat)row.pop_back();check(fabsl(determinant(mat)-det)<=max(1.0L,fabsl(det))*1e-10L);
    }
    vector<long double> x;check(gaussian_elimination({{1,1,2},{2,2,4}},x)==2);check(gaussian_elimination({{1,1,2},{2,2,5}},x)==0);
}

void geometry() {
    for(int trial=0;trial<1200;++trial) {
        vector<Point> points;for(int i=0,n=rnd(40);i<n;++i)points.push_back({rnd(41)-20,rnd(41)-20});
        auto hull=convex_hull(points);i128 farthest=0;
        for(Point a:points)for(Point b:points)farthest=max(farthest,squared_distance(a,b));
        check(convex_diameter_squared(hull)==farthest);reverse(hull.begin(),hull.end());check(convex_diameter_squared(hull)==farthest);
        for(Point p:points)if(hull.size()>=3) {
            vector<DPoint> polygon;for(Point h:hull)polygon.push_back({(long double)h.x,(long double)h.y});
            check(point_in_polygon(polygon,{(long double)p.x,(long double)p.y})!=0);
        }
        DPoint a{(long double)rnd(21),(long double)rnd(21)}, b{(long double)rnd(21),(long double)rnd(21)};
        if(norm(a-b)>0) {
            DPoint p{(long double)rnd(21),(long double)rnd(21)};DPoint q=projection_on_line(p,a,b);
            check(fabsl(cross(q-a,b-a))<1e-7L);check(fabsl(dot(p-q,b-a))<1e-7L);
            check(fabsl(norm(p-q)-distance_to_line(p,a,b))<1e-7L);
        }
    }
    for(int x=-3;x<=3;++x)for(int y=-3;y<=3;++y) {
        auto c=circle_intersections({{0,0},2},{{(long double)x,(long double)y},1});
        check(bool(c));for(auto p:*c) {check(fabsl(norm(p)-2)<1e-8L);check(fabsl(norm(p-DPoint{(long double)x,(long double)y})-1)<1e-8L);}
    }
    check(!circle_intersections({{0,0},1},{{0,0},1}));
    check(circle_intersections({{0,0},1},{{2,0},1})->size()==1);
    check(circle_intersections({{0,0},1},{{3,0},1})->empty());
    Point3D p{3,4,7},normal{0,0,2};auto q=project_on_plane(p,{0,0,0},normal);
    check(q.x==3&&q.y==4&&q.z==0);check(distance_to_plane(p,{0,0,0},normal)==7);
}

void techniques() {
    for(int trial=0;trial<1000;++trial) {
        int n=rnd(65);vector<ll>a(n);for(ll& x:a)x=rnd(101)-50;
        PrefixSum prefix(a);Compressor<ll> comp(a);
        vector<MoQuery> queries;vector<ll> expected;
        for(int i=0;i<40;++i) {
            int l=rnd(n+1),r=rnd(n+1);if(l>r)swap(l,r);
            ll sum=accumulate(a.begin()+l,a.begin()+r,0LL);check(prefix.query(l,r)==sum);
            queries.push_back({l,r,i});expected.push_back(set<ll>(a.begin()+l,a.begin()+r).size());
        }
        unordered_map<ll,int> freq;ll distinct=0;
        auto actual=mos_algorithm(n,queries,[&](int i){if(freq[a[i]]++==0)++distinct;},[&](int i){if(--freq[a[i]]==0)--distinct;},[&](){return distinct;});
        check(actual==expected);for(ll x:a)check(comp.value(comp.id(x))==x);
        ll limit=rnd(101)-50;int longest=0;
        for(int i=0;i<n;++i) {ll sum=0;for(int j=i;j<n;++j){sum+=a[j];if(sum<=limit)longest=max(longest,j-i+1);}}
        check(longest_window_signed(a,limit)==longest);
        vector<ll> previous(n+1),current(n+1),want(n+1,LINF);
        for(ll& x:previous)x=rnd(100);
        auto cost=[](int j,int i)->ll{return (i-j)*(i-j);};
        compute_divide_conquer(0,n,0,n,previous,current,cost);
        for(int i=0;i<=n;++i)for(int j=0;j<=i;++j)want[i]=min(want[i],previous[j]+cost(j,i));
        check(current==want);
    }
    for(int n=0;n<100;++n) {
        int k=triangular_state_limit(n);check(k*(k+1)/2<=n&&(k+1)*(k+2)/2>n);
        auto dp=triangular_bound_dp(n);check(dp[n][0]==0);
    }
    for(ll value:{numeric_limits<ll>::min(),-1234567LL,0LL,1LL,1234567LL,numeric_limits<ll>::max()}) {
        for(int base=2;base<=36;++base) {
            auto [negative,digits]=to_base(value,base);i128 reconstructed=0;
            for(int d:digits)reconstructed=reconstructed*base+d;
            if(negative)reconstructed=-reconstructed;check(reconstructed==value);
            if(value>=0)check(from_base(to_base_string(value,base),base)==value);
        }
    }
    int value=0;check(parse_int("-2147483648",value)&&value==numeric_limits<int>::min());
    check(!parse_int("2147483648",value)&&!parse_int("3x",value)&&!parse_int(" 3",value));
}

// Regression reproducers and outstanding findings from the book audit.
void core_examples() {
    for(int trial=0;trial<1000;++trial) {
        long double angle=(rnd(20001)-10000)/100.0L;
        check(normalize_angle(angle)>=0&&normalize_angle(angle)<2*PI);
        check(fabsl(signed_angle(angle))<=PI);
        long double x=rnd(2001)-1000,y=rnd(2001)-1000;
        auto [rx,ry]=rotate_coordinates(x,y,angle);
        check(fabsl(hypotl(rx,ry)-hypotl(x,y))<1e-8L);
        check(fabsl(angle_distance(angle,angle+2*PI))<1e-8L);
        check(fabsl(radians_to_degrees(degrees_to_radians(angle))-angle)<1e-8L);
        vector<int> a={rnd(9),rnd(9),rnd(9)};auto b=a;normalize(b);
        check(is_sorted(b.begin(),b.end())&&adjacent_find(b.begin(),b.end())==b.end());
        check(sum_values(a)==accumulate(a.begin(),a.end(),0LL));
        common_algorithms(a);ordered_range_algorithms(a,3);
    }
    check(promoted_product(2000000000,2000000000)==4000000000000000000LL);
    check(count_bits(1ULL<<63)==1&&most_significant_bit_index(1ULL<<63)==63);
    auto old=cout.rdbuf();ostringstream captured;cout.rdbuf(captured.rdbuf());
    vector_and_string_examples();associative_container_examples();queue_examples();
    aggregate_types_example();bitset_example();sequence_algorithms();
    i128 smallest=-(i128(1)<<126);smallest+=smallest;print_i128(smallest);
    cout.rdbuf(old);
    check(captured.str().find("-170141183460469231731687303715884105728")!=string::npos);
}

void finding(const string& name) {
    if(name=="odt_empty") {
        DisjointIntervals odt;odt.intervals[0]={10,7};odt.assign(3,3,99);
        auto it=prev(odt.intervals.upper_bound(4));
        cout<<"assign(3,3,99): containing interval="<<it->first<<','<<it->second.right<<" value="<<it->second.value<<endl;
        check(it->first<=4&&4<it->second.right&&it->second.value==7);
    } else if(name=="bcc_loop") {
        vector<vector<pair<int,int>>> graph(1);graph[0]={{0,0},{0,0}};
        VertexBiconnectedComponents bcc(graph);cout<<"one vertex with self-loop: blocks="<<bcc.components.size()<<endl;
        check(bcc.components==vector<vector<int>>{{0}});
    } else if(name=="matrix_mod_one") {
        auto result=matrix_power({{42}},0,1);cout<<"A^0 mod 1="<<result[0][0]<<endl;check(result[0][0]==0);
    } else if(name=="dijkstra_large") {
        vector<vector<pair<int,ll>>> g(2);g[0]={{1,LINF+1}};
        auto d=dijkstra(g,0);cout<<"edge="<<LINF+1<<" result="<<d[1]<<" (LINF="<<LINF<<')'<<endl;check(d[1]==LINF+1);
    } else if(name=="window_threshold") {
        vector<ll>a{0};ll limit=numeric_limits<ll>::min();cout<<"window {0}, limit LLONG_MIN"<<endl;
        check(longest_window_signed(a,limit)==0);
    } else if(name=="floyd_negative_overflow") {
        vector<vector<ll>> d(40,vector<ll>(40,-1));
        for(int i=0;i<40;++i)d[i][i]=0;
        cout<<"40 vertices, all non-diagonal edges -1, diagonal 0"<<endl;
        floyd_warshall(d);check(negative_cycle_between(d,0,2));
    } else if(name=="point_subtraction") {
        Point a{numeric_limits<ll>::min(),0},b{1,0};cout<<"distance LLONG_MIN -> 1"<<endl;
        i128 want=(i128)b.x-a.x;want*=want;check(squared_distance(a,b)==want);
    } else if(name=="binary_int_max") {
        cout<<"first_true [INT_MAX,INT_MAX], false"<<endl;
        check(audit_integer_search(numeric_limits<int>::max(),numeric_limits<int>::max(),numeric_limits<int>::max(),true)==ll(numeric_limits<int>::max())+1);
    } else if(name=="binary_wide_domain") {
        cout<<"first_true [INT_MIN,INT_MAX], threshold 0"<<endl;
        check(audit_integer_search(numeric_limits<int>::min(),numeric_limits<int>::max(),0)==0);
    } else if(name=="base_roundtrip") {
        auto s=to_base_string(-42,16);cout<<"to_base_string(-42,16)="<<s<<endl;check(from_base(s,16)==-42);
    } else if(name=="divide_conquer_infinite") {
        vector<ll> prev{LINF},next(1);cout<<"D&C: one unreachable state"<<endl;
        compute_divide_conquer(0,0,0,0,prev,next,[](int,int){return 0LL;});check(next[0]==LINF);
    } else if(name=="flow_reverse_index") {
        Dinic flow(1);flow.add_edge(0,0,7);
        auto e=flow.graph[0][0];cout<<"self-loop reverse="<<e.reverse<<" reverse capacity="<<flow.graph[e.to][e.reverse].capacity<<endl;
        check(e.reverse==1&&flow.graph[e.to][e.reverse].reverse==0);
    } else if(name=="stl_empty") {
        cout<<"common_algorithms(empty vector)"<<endl;common_algorithms({});
    } else if(name=="mcmf_disconnected_cycle") {
        MinCostMaxFlow f(4);f.add_edge(0,1,1,0);f.add_edge(2,3,1,-1);f.add_edge(3,2,1,0);
        auto result=f.run(0,1,1);
        cout<<"flow="<<result.first<<" cost="<<result.second<<"; feasible flow+circulation has cost -1"<<endl;
        check(result.first==1&&result.second==-1);
    } else {cerr<<"Unknown case "<<name<<'\n';exit(2);}
}

void benchmark(const string& name) {
    int n=200000;
    if(name=="bench_structures") {
        SegmentTree seg(n);ll checksum=0;
        for(int i=0;i<1000000;++i) {int p=rnd(n);seg.set_value(p,i);checksum+=seg.query(p,min(n,p+100));}
        cout<<"n=200000; 1000000 point updates and range queries; checksum="<<checksum<<'\n';
    } else if(name=="bench_scc") {
        vector<vector<int>> graph(n);for(int i=1;i<n;++i) {graph[i].push_back(rnd(i));graph[i].push_back(rnd(i));}
        auto c=strongly_connected_components(graph);check(*max_element(c.begin(),c.end())+1==n);
        cout<<"DAG V=200000,E=399998; SCC="<<n<<'\n';
    } else if(name=="bench_ntt") {
        vector<int>a(1<<18,1);auto c=convolution(a,a);check(c[(1<<18)-1]==1<<18);
        cout<<"NTT two arrays n=262144; coefficients="<<c.size()<<'\n';
    } else if(name=="bench_suffix") {
        string s(n,'a');for(char& c:s)c+=rnd(4);
        auto sa=suffix_array(s);auto lcp=lcp_array(s,sa);check((int)lcp.size()==n-1);
        cout<<"suffix array + LCP, n=200000, alphabet=4\n";
    } else if(name=="bench_dijkstra") {
        vector<vector<pair<int,ll>>> graph(n);
        for(int i=0;i<n;++i) {
            if(i+1<n)graph[i].push_back({i+1,1});
            graph[i].push_back({rnd(n),rnd(1000)});
        }
        auto d=dijkstra(graph,0);check(d.back()<LINF);cout<<"Dijkstra V=200000,E=399999; d[last]="<<d.back()<<'\n';
    } else {
        vector<vector<int>> tree(n);
        for(int i=1;i<n;++i) {int p=(name=="bench_trees")?rnd(i):i-1;tree[p].push_back(i);tree[i].push_back(p);}
        if(name=="stack_hld"||name=="bench_trees") {HLD hld(tree);check(hld.size[0]==n);}
        else if(name=="stack_lowlink") {LowLink low(n);for(int i=1;i<n;++i)low.add_edge(i-1,i,i-1);low.run();check((int)low.bridges.size()==n-1);}
        else if(name=="stack_reroot") {DistanceRerooting rr(tree);check(rr.distance_sum[0]==(ll)n*(n-1)/2);}
        else if(name=="stack_euler_lca") {EulerRMQLCA lca(tree);check(lca.distance(0,n-1)==n-1);}
        else {cerr<<"Unknown benchmark\n";exit(2);}
        cout<<name<<" n="<<n<<'\n';
    }
}

int main(int argc,char** argv) {
    string name=argc>1?argv[1]:"";
    if(name=="structures")structures();
    else if(name=="graphs")graphs();
    else if(name=="trees")trees();
    else if(name=="numeric")numeric();
    else if(name=="geometry")geometry();
    else if(name=="techniques")techniques();
    else if(name=="core")core_examples();
    else if(name.rfind("bench_",0)==0||name.rfind("stack_",0)==0)benchmark(name);
    else finding(name);
    cout<<"OK "<<name<<": "<<audit_checks<<" checks\n";
}
