#!/usr/bin/env python3
"""Test the printed feasible-partition example against exhaustive cut enumeration."""
import argparse
from pathlib import Path
import subprocess
import tempfile
from audit_book import HEADER, LISTING, ROOT

TEST = r'''
mt19937 rng(20261028);
int cases = 0;
void check(const vector<int>& a) {
    int n = a.size(); vector<ll> prefix(n+1);
    for(int i=0;i<n;++i)prefix[i+1]=prefix[i]+a[i];
    auto cost=[&](int j,int i)->ll {ll sum=prefix[i]-prefix[j];return sum*sum;};
    // Nonnegative elements make squared segment sums a Monge cost.
    vector<ll> want(n+1,LINF);
    if(!n)want[0]=0;
    else for(int mask=0;mask<(1<<(n-1));++mask) {
        int last=0,groups=1;ll value=0;
        for(int i=1;i<n;++i)if(mask>>(i-1)&1){value+=cost(last,i);last=i;++groups;}
        value+=cost(last,n);want[groups]=min(want[groups],value);
    }
    for(int groups=-1;groups<=n+1;++groups) {
        ll expected=groups<0||groups>n?LINF:want[groups];
        assert(partition_nonempty(n,groups,cost)==expected);++cases;
    }
}
int main() {
    for(int n=0;n<=7;++n) {
        int states=1;for(int i=0;i<n;++i)states*=3;
        for(int state=0;state<states;++state){int x=state;vector<int>a(n);for(int&v:a){v=x%3;x/=3;}check(a);}
    }
    for(int t=0;t<1000;++t){vector<int>a(rng()%13);for(int&v:a)v=rng()%100;check(a);}
    // Count transitions to verify that impossible prefixes do not trigger fallback.
    ll calls=0;int n=100000,groups=20;
    ll result=partition_nonempty(n,groups,[&](int j,int i)->ll {++calls;return (ll)(i-j)*(i-j);});
    int q=n/groups,r=n%groups;
    assert(result==(ll)(groups-r)*q*q+(ll)r*(q+1)*(q+1));
    assert(calls < 40LL*groups*n); // Far below quadratic work per layer.
    cout<<"OK: "<<cases<<" partition queries vs exhaustive cuts; empty, zero/invalid groups, duplicates and random arrays; n="<<n<<", groups="<<groups<<", transitions="<<calls<<'\n';
}
'''


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--compiler',default='clang++')
    p.add_argument('--cxxflag',action='append',default=[])
    args=p.parse_args()
    source=HEADER+'\n'.join(b[2] for b in LISTING.finditer((ROOT/'dp/divide_conquer_optimization.tex').read_text()))+TEST
    with tempfile.TemporaryDirectory(prefix='handbook-dc-partition-') as tmp:
        cpp,exe=Path(tmp)/'test.cpp',Path(tmp)/'test';cpp.write_text(source)
        subprocess.run([args.compiler,'-std=c++17','-O2',*args.cxxflag,str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True,timeout=90)


if __name__=='__main__':main()
