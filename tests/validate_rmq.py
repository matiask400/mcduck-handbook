#!/usr/bin/env python3
"""Check the actual inclusive SparseTableMin and optionally benchmark its predecessor."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import statistics
import subprocess
import sys
import tempfile
from audit_book import HEADER, LISTING, ROOT

TEST=r'''
mt19937_64 rmq_rng(20261027);
long long arrays_checked=0,queries_checked=0;
void check_array(const vector<ll>& a){
    auto saved=a;after::SparseTableMin rmq(a);assert(a==saved);
    for(int l=0;l<(int)a.size();++l){
        ll answer=numeric_limits<ll>::max();
        for(int r=l;r<(int)a.size();++r){
            answer=min(answer,a[r]);assert(rmq.query(l,r)==answer);++queries_checked;
        }
    }
    ++arrays_checked;
}
void correctness(){
    for(int n=0;n<=8;++n){
        int states=1;for(int i=0;i<n;++i)states*=3;
        for(int code=0;code<states;++code){
            int x=code;vector<ll>a(n);for(ll& v:a){v=x%3-1;x/=3;}check_array(a);
        }
    }
    check_array({numeric_limits<ll>::min(),numeric_limits<ll>::max(),0,
                 numeric_limits<ll>::min(),-1,1});
    for(int t=0;t<1500;++t){
        vector<ll>a(rmq_rng()%129);
        for(ll& v:a)v=ll(rmq_rng()%20000000001ULL)-10000000000LL;
        check_array(a);
    }
    // Powers of two, their neighbors, full range, singletons and suffixes.
    for(int k=0;k<=16;++k)for(int d=-1;d<=1;++d){
        int n=(1<<k)+d;if(n<=0)continue;
        vector<ll>a(n);for(int i=0;i<n;++i)a[i]=ll(n-i);
        after::SparseTableMin rmq(a);
        for(int len=1;len<=n;len*=2){
            assert(rmq.query(0,len-1)==a[len-1]);
            assert(rmq.query(n-len,n-1)==1);queries_checked+=2;
        }
        assert(rmq.query(0,n-1)==1);assert(rmq.query(n-1,n-1)==1);queries_checked+=2;
    }
    cout<<"OK: "<<arrays_checked<<" arrays; "<<queries_checked
        <<" inclusive queries vs direct minima; empty/singleton, duplicates, negatives, LLONG limits and power-of-two boundaries\n";
}
using Clock=chrono::steady_clock;
template<class RMQ>void benchmark(int n,bool short_ranges){
    vector<ll>a(n);for(ll& x:a)x=ll(rmq_rng()%1000000)-500000;
    vector<pair<int,int>> queries(1000000);
    for(auto& [l,r]:queries){
        l=rmq_rng()%n;
        if(short_ranges)r=min(n-1,l+int(rmq_rng()%16));
        else{r=rmq_rng()%n;if(l>r)swap(l,r);}
    }
    auto start=Clock::now();RMQ rmq(a);
    double build=chrono::duration<double,milli>(Clock::now()-start).count();
    start=Clock::now();uint64_t hash=0;
    for(auto [l,r]:queries)hash=(hash^uint64_t(rmq.query(l,r)))*1099511628211ULL;
    double query=chrono::duration<double,milli>(Clock::now()-start).count();
    size_t cells=0;for(const auto& row:rmq.table)cells+=row.size();
    cout<<setprecision(12)<<"{\"build_ms\":"<<build<<",\"query_ms\":"<<query
        <<",\"total_ms\":"<<build+query<<",\"table_cells\":"<<cells
        <<",\"checksum\":\""<<hash<<"\"}\n";
}
int main(int argc,char** argv){
    if(argc==1){correctness();return 0;}
    int n=string(argv[1])=="large"?1000000:200000;bool short_ranges=string(argv[1])=="short";
    if(string(argv[2])=="before")benchmark<before::SparseTableMin>(n,short_ranges);
    else benchmark<after::SparseTableMin>(n,short_ranges);
}
'''


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',default='clang++')
    p.add_argument('--cxxflag',action='append',default=[])
    p.add_argument('--baseline-ref',default='cd5adda')
    p.add_argument('--benchmark',action='store_true')
    p.add_argument('--repeat',type=int,default=5)
    p.add_argument('--json-output',type=Path)
    a=p.parse_args();name='data_structures/sparse_table.tex';code=HEADER;lines={}
    current=(ROOT/name).read_text()
    for version in ('before','after'):
        raw=subprocess.check_output(['git','show',a.baseline_ref+':'+name],cwd=ROOT,text=True) if version=='before' else current
        snippet=next(LISTING.finditer(raw))[2];code+=f'\nnamespace {version} {{\n{snippet}\n}}\n'
        lines[version]=sum(bool(x.strip()) for x in re.sub(r'//[^\n]*','',snippet).splitlines())
    code+=TEST
    report=dict(baseline_ref=a.baseline_ref,compiler=subprocess.check_output([a.compiler,'--version'],text=True).splitlines()[0],
                platform=platform.platform(),flags=['-std=c++17','-O2',*a.cxxflag],seed=20261027,
                active_code_lines=lines,source_sha256=hashlib.sha256(current.encode()).hexdigest(),benchmarks=[])
    worker=r'''
import json,resource,subprocess,sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True,check=True,timeout=30)
result=json.loads(r.stdout)
result['peak_rss_mib']=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss/(1024**2 if sys.platform=='darwin' else 1024)
print(json.dumps(result))
'''
    with tempfile.TemporaryDirectory(prefix='handbook-rmq-') as tmp:
        cpp,exe=Path(tmp)/'test.cpp',Path(tmp)/'test';cpp.write_text(code)
        subprocess.run([a.compiler,*report['flags'],str(cpp),'-o',str(exe)],check=True)
        r=subprocess.run([str(exe)],text=True,capture_output=True,check=True,timeout=60)
        report['correctness']=r.stdout.strip();print(r.stdout.strip(),flush=True)
        if a.benchmark:
            for case in ('random','large','short'):
                samples={'before':[],'after':[]}
                for i in range(a.repeat):
                    for version in (('before','after') if i%2==0 else ('after','before')):
                        r=subprocess.run([sys.executable,'-c',worker,str(exe),case,version],text=True,capture_output=True,check=True)
                        samples[version].append(json.loads(r.stdout))
                assert len({x['checksum'] for runs in samples.values() for x in runs})==1
                row=dict(case=case,n=1000000 if case=='large' else 200000,queries=1000000,samples=samples)
                for version,runs in samples.items():
                    row[version]={key:statistics.median(r[key] for r in runs) for key in ('build_ms','query_ms','total_ms')}
                    row[version]['peak_rss_mib']=max(r['peak_rss_mib'] for r in runs)
                    row[version]['table_cells']=runs[0]['table_cells']
                report['benchmarks'].append(row);print(json.dumps({k:v for k,v in row.items() if k!='samples'}),flush=True)
    if a.json_output:a.json_output.write_text(json.dumps(report,indent=2)+'\n')
    print('Active C++ lines:',lines)


if __name__=='__main__':main()
