mt19937_64 rng_sa_scc(20261027);
int pick(int n){return int(rng_sa_scc()%n);}
long long string_cases=0,graph_cases=0,sat_cases=0;

void check_string(const string& s){
    vector<int> expected(s.size());iota(expected.begin(),expected.end(),0);
    sort(expected.begin(),expected.end(),[&](int a,int b){
        return lexicographical_compare(s.begin()+a,s.end(),s.begin()+b,s.end(),
            [](char x,char y){return (unsigned char)x<(unsigned char)y;});
    });
    auto sa=after::suffix_array(s);assert(sa==expected);
    auto lcp=after::lcp_array(s,sa);assert(lcp.size()==(s.empty()?0:s.size()-1));
    for(int i=0;i<(int)lcp.size();++i){
        int length=0;while(sa[i]+length<(int)s.size()&&sa[i+1]+length<(int)s.size()
            &&s[sa[i]+length]==s[sa[i+1]+length])++length;
        assert(lcp[i]==length);
    }
    ++string_cases;
}

void check_graph(const vector<vector<int>>& graph){
    int n=graph.size();auto comp=after::strongly_connected_components(graph);
    vector<vector<bool>> reach(n,vector<bool>(n));
    for(int v=0;v<n;++v){reach[v][v]=true;for(int to:graph[v])reach[v][to]=true;}
    for(int k=0;k<n;++k)for(int i=0;i<n;++i)for(int j=0;j<n;++j)
        reach[i][j]=reach[i][j]||(reach[i][k]&&reach[k][j]);
    set<int> ids(comp.begin(),comp.end());
    if(n)assert(*ids.begin()==0&&*ids.rbegin()==(int)ids.size()-1);
    for(int i=0;i<n;++i){
        for(int j=0;j<n;++j)assert((comp[i]==comp[j])==(reach[i][j]&&reach[j][i]));
        for(int j:graph[i])assert(comp[i]==comp[j]||comp[i]<comp[j]);
    }
    ++graph_cases;
}

void correctness(){
    vector<unsigned char> alphabet{0,1,128,255};
    for(int n=0;n<=7;++n){
        int states=1<<(2*n);for(int mask=0;mask<states;++mask){
            string s(n,'\0');for(int i=0;i<n;++i)s[i]=char(alphabet[mask>>(2*i)&3]);check_string(s);
        }
    }
    for(int t=0;t<5000;++t){string s(pick(100),'\0');for(char& c:s)c=char(pick(256));check_string(s);}
    for(int n=1;n<=300;++n)check_string(string(n,'\0'));
    string bytes;for(int i=0;i<256;++i)bytes+=char(i);check_string(bytes);check_string(bytes+bytes);
    for(int n=0;n<=4;++n)for(unsigned mask=0;mask<(1U<<(n*n));++mask){
        vector<vector<int>> graph(n);
        for(int i=0;i<n;++i)for(int j=0;j<n;++j)if(mask>>(i*n+j)&1U)graph[i].push_back(j);
        check_graph(graph);
    }
    for(int t=0;t<2000;++t){
        int n=1+pick(25);vector<vector<int>> graph(n);
        for(int i=0,m=pick(150);i<m;++i)graph[pick(n)].push_back(pick(n));check_graph(graph);
    }
    vector<vector<int>> chain(4096);for(int i=0;i<4095;++i)chain[i].push_back(i+1);
    auto ids=after::strongly_connected_components(chain);for(int i=0;i<4096;++i)assert(ids[i]==i);
    chain.back().push_back(0);ids=after::strongly_connected_components(chain);
    assert(all_of(ids.begin(),ids.end(),[](int id){return id==0;}));
    assert(after::strongly_connected_components({}).empty());
    assert(after::TwoSAT(0).solve()->empty());
    for(int trial=0;trial<10000;++trial){
        int n=1+pick(7);after::TwoSAT sat(n);vector<tuple<int,bool,int,bool>> clauses;
        auto clause=[&](int x,bool a,int y,bool b){clauses.emplace_back(x,a,y,b);};
        for(int op=0,m=pick(30);op<m;++op){
            int x=pick(n),y=pick(n),type=pick(6);bool a=pick(2),b=pick(2);
            if(type==0){sat.add_or(x,a,y,b);clause(x,a,y,b);}
            if(type==1){sat.force(x,a);clause(x,a,x,a);}
            if(type==2){sat.imply(x,a,y,b);clause(x,!a,y,b);}
            if(type==3){sat.add_xor(x,a,y,b);clause(x,a,y,b);clause(x,!a,y,!b);}
            if(type==4){sat.add_equal(x,y);clause(x,false,y,true);clause(y,false,x,true);}
            if(type==5){sat.add_different(x,y);clause(x,true,y,true);clause(x,false,y,false);}
        }
        auto valid=[&](int mask){for(auto [x,a,y,b]:clauses)if(bool(mask>>x&1)!=a&&bool(mask>>y&1)!=b)return false;return true;};
        bool possible=false;for(int mask=0;mask<(1<<n);++mask)possible|=valid(mask);
        auto answer=sat.solve();assert(bool(answer)==possible);
        if(answer){int mask=0;for(int i=0;i<n;++i)if((*answer)[i])mask|=1<<i;assert(valid(mask));}
        ++sat_cases;
    }
    cout<<"OK: "<<string_cases<<" byte-string SA/LCP cases; "<<graph_cases
        <<" graphs vs reachability + topological IDs; 4096-vertex chain/cycle; "
        <<sat_cases<<" formulas using every 2-SAT helper vs all assignments\n";
}

uint64_t digest(uint64_t h,int x){return (h^uint64_t(x))*1099511628211ULL;}
template<class T>uint64_t hash_values(const vector<T>& values){uint64_t h=0;for(auto x:values)h=digest(h,x);return h;}
void benchmark(string name,bool old){
    using Clock=chrono::steady_clock;uint64_t checksum=0;double ms;
    if(name.rfind("suffix_",0)==0){
        string s(500000,'a');if(name=="suffix_random")for(char& c:s)c+=pick(4);
        vector<int> sa,lcp;auto start=Clock::now();
        if(old){sa=before::suffix_array(s);lcp=before::lcp_array(s,sa);}
        else{sa=after::suffix_array(s);lcp=after::lcp_array(s,sa);}
        ms=chrono::duration<double,milli>(Clock::now()-start).count();checksum=hash_values(sa)^hash_values(lcp);
    }else{
        int n=200000;vector<vector<int>> graph(n);
        if(name=="scc_dag")for(int i=1;i<n;++i){graph[i].push_back(pick(i));graph[i].push_back(pick(i));}
        else for(int i=0;i<n;++i){int base=i/100*100;graph[i].push_back(base+(i+1)%100);graph[i].push_back(base+pick(100));}
        auto start=Clock::now();auto ids=old?before::strongly_connected_components(graph):after::strongly_connected_components(graph);
        ms=chrono::duration<double,milli>(Clock::now()-start).count();
        vector<int> smallest(n,n);for(int i=0;i<n;++i)smallest[ids[i]]=min(smallest[ids[i]],i);
        for(int i=0;i<n;++i)checksum=digest(checksum,smallest[ids[i]]);
    }
    cout<<setprecision(12)<<"{\"ms\":"<<ms<<",\"checksum\":\""<<checksum<<"\"}\n";
}
int main(int argc,char** argv){if(argc==1)correctness();else benchmark(argv[1],string(argv[2])=="before");}
