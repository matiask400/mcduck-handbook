#!/usr/bin/env python3
"""Compare real handbook and pinned KACTL code using identical C++ workloads.

No handbook code is modified. KACTL is read from an external checkout; the
few configuration substitutions required for equivalent operations are logged.
"""
import argparse
import hashlib
import json
import platform
from pathlib import Path
import re
import statistics
import subprocess
import sys
import tempfile
from audit_book import ROOT, LISTING, SELECTION

OUR_FILES={
 'data_structures/dsu.tex','data_structures/fenwick.tex','data_structures/segment_tree.tex',
 'data_structures/sparse_table.tex','data_structures/cht_li_chao.tex',
 'graphs/dag_and_scc.tex','graphs/flows.tex','graphs/matching.tex','graphs/trees_lca.tex',
 'graphs/lca_euler_rmq.tex','graphs/shortest_paths.tex','geometry/geometry_2d.tex',
 'geometry/lines_circles_polygons.tex','math/gcd_and_modular.tex',
 'math/miller_rabin_pollard_rho.tex','math/ntt.tex',
}
EXTRA=[('strings/suffix_array_lcp.tex',0),('strings/manacher.tex',0),
       ('strings/prefix_and_z.tex',0),('strings/prefix_and_z.tex',3),
       ('strings/aho_corasick.tex',0),('dp/lis.tex',0),('graphs/konig_vertex_cover.tex',0)]
KFILES={
 'kbit':['data-structures/FenwickTree.h'],
 'ksegment':['data-structures/SegmentTree.h'],
 'krmq':['data-structures/RMQ.h'], 'kdsu':['data-structures/UnionFind.h'],
 'kscc':['graph/SCC.h'], 'ktopo':['graph/TopoSort.h'],
 'klift':['graph/BinaryLifting.h'], 'klca':['graph/LCA.h'],
 'kflow':['graph/Dinic.h'], 'kmatch':['graph/HopcroftKarp.h'],
 'khungarian':['graph/WeightedMatching.h'],
 'ksuffix':['strings/SuffixArray.h'], 'kmanacher':['strings/Manacher.h'],
 'kkmp':['strings/KMP.h'], 'kz':['strings/Zfunc.h'], 'kaho':['strings/AhoCorasick.h'],
 'kntt':['numerical/NumberTheoreticTransform.h'], 'klis':['various/LIS.h'],
 'khull':['geometry/ConvexHull.h','geometry/HullDiameter.h'],
 'kfactor':['number-theory/Factor.h'], 'kline':['data-structures/LineContainer.h'],
 'kfloyd':['graph/FloydWarshall.h'], 'kbellman':['graph/BellmanFord.h'],
 'kcover':['graph/MinimumVertexCover.h'],
 'kmcmf':['graph/MinCostMaxFlow.h'],
}
CASES=['fenwick','fenwick_lower_bound','segment_sum','sparse_rmq','dsu','scc','scc_cycles','topological',
       'lca_binary','lca_rmq','dinic','hopcroft_karp','hungarian',
       'suffix_random','suffix_repeated','manacher','kmp','z','aho_random','aho_suffixes',
       'ntt','ntt_boundary','lis','convex_hull','calipers','line_queries',
       'miller_rabin','miller_rabin_simple','factorization','factorization_simple',
       'floyd','bellman_ford','vertex_cover','min_cost_flow']
HEADER=r'''
#include <bits/stdc++.h>
#include <bits/extc++.h>
using namespace std;
using ll=long long;
using i128=__int128_t;
using vi=vector<int>;
using pii=pair<int,int>;
const ll LINF=numeric_limits<ll>::max()/4;
const int INF=numeric_limits<int>::max()/4;
#define rep(i,a,b) for(int i=(a);i<(b);++i)
#define sz(x) (int)(x).size()
#define fore(i,a,b) for(int i=(a),cmp_end=(b);i<cmp_end;++i)
#define SZ(x) (int)(x).size()
'''


def strip_comments(code):
    return re.sub(r'//[^\n]*|/\*.*?\*/','',code,flags=re.S)


def metrics(code):
    clean=strip_comments(code)
    lines=[x for x in clean.splitlines() if x.strip() and not x.lstrip().startswith('#')]
    return dict(code_lines=len(lines),code_chars=sum(len(x.strip()) for x in lines))


def source(kroot):
    text=HEADER+'\nnamespace ours {\n';our={};kmetadata={};filehash={}
    selected=[]
    for name,index,stop in SELECTION:
        if name not in OUR_FILES:continue
        if name=='graphs/trees_lca.tex' and index==0:continue
        if name=='geometry/lines_circles_polygons.tex' and index!=6:continue
        if name=='graphs/shortest_paths.tex' and index not in (3,5):continue
        selected.append((name,index,stop))
    for name,index in EXTRA:selected.append((name,index,None))
    for name,index,stop in selected:
        raw=(ROOT/name).read_text();code=list(LISTING.finditer(raw))[index][2]
        if stop:code=code[:code.index(stop)]
        if name=='graphs/konig_vertex_cover.tex':code=code[:code.index('\nvoid konig_example')]
        text+='\n'+code+'\n';our.setdefault(name,{})[str(index)]=metrics(code)
        filehash[name]=hashlib.sha256(raw.encode()).hexdigest()
    text+='}\n#define all(x) (x).begin(),(x).end()\n'
    for namespace,files in KFILES.items():
        seen=set();dependencies={}
        def expand(relative):
            path=(kroot/'content'/relative).resolve()
            if path in seen:return ''
            seen.add(path);raw=path.read_text();dependencies[str(path.relative_to(kroot))]=metrics(raw)
            raw=re.sub(r'^#pragma once\s*$','',raw,flags=re.M)
            def include(m):
                return expand(str((path.parent/m[1]).relative_to(kroot/'content')))
            return re.sub(r'^#include "([^"]+)"\s*$',include,raw,flags=re.M)
        block='\n'.join(expand(f) for f in files);adapt=[]
        if namespace=='ksegment':
            block=block.replace('typedef int T;','typedef ll T;').replace('unit = INT_MIN','unit = 0').replace('return max(a, b);','return a+b;')
            adapt=['Documented T/f/unit configuration: ll, addition, identity 0.']
        if namespace=='kaho':
            block=block.replace("first = 'A'","first = 'a'")
            adapt=['Documented alphabet configuration: lowercase a..z.']
        if namespace=='kntt':
            block=block.replace('const ll mod = 1000000007;','const ll mod = 998244353;')
            block=block.replace('const ll mod = (119 << 23) + 1, root = 62;','const ll root = 62;')
            adapt=['Set ModPow.h modulus to NTT modulus 998244353 and remove duplicate mod declaration.']
        text+=f'\nnamespace {namespace} {{\n{block}\n}}\n'
        kmetadata[namespace]=dict(files=files,dependencies=dependencies,total=metrics(block),adaptations=adapt)
    text+=(ROOT/'tests/compare_kactl_test.cpp').read_text()
    return text,dict(our_code_metrics=our,kactl_code_metrics=kmetadata,our_file_sha256=filehash)


def measured(exe,case,provider):
    # Independent worker avoids cumulative RUSAGE_CHILDREN from earlier runs.
    worker=r'''
import json,resource,subprocess,sys
try:
 r=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=90)
 rss=resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
 print(json.dumps(dict(exit_code=r.returncode,peak_rss_mib=rss/(1024**2 if sys.platform=='darwin' else 1024),
  result=json.loads(r.stdout) if r.returncode==0 else None,error=r.stderr[-2000:])))
except subprocess.TimeoutExpired:print(json.dumps(dict(timeout=True)))
'''
    r=subprocess.run([sys.executable,'-c',worker,str(exe),case,provider],text=True,capture_output=True,check=True)
    return json.loads(r.stdout)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('kactl',type=Path);p.add_argument('--compiler',default='g++')
    p.add_argument('--cxxflag',action='append',default=[])
    p.add_argument('--repeat',type=int,default=5)
    p.add_argument('--case',action='append',choices=CASES)
    p.add_argument('--json-output',type=Path,required=True)
    p.add_argument('--compile-only',action='store_true')
    a=p.parse_args();code,metadata=source(a.kactl.resolve())
    report=dict(**metadata,compiler=subprocess.check_output([a.compiler,'--version'],text=True).splitlines()[0],
                platform=platform.platform(),
                kactl_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=a.kactl,text=True).strip(),
                seed=20261026,flags=['-std=c++17','-O2',*a.cxxflag],
                source_sha256=hashlib.sha256(code.encode()).hexdigest(),results=[])
    with tempfile.TemporaryDirectory(prefix='handbook-kactl-compare-') as tmp:
        cpp,exe=Path(tmp)/'test.cpp',Path(tmp)/'test';cpp.write_text(code)
        subprocess.run([a.compiler,*report['flags'],str(cpp),'-o',str(exe)],check=True)
        if a.compile_only:print('comparison compilation OK');return
        small=subprocess.run([str(exe),'verify','both'],capture_output=True,text=True,check=True)
        report['verification']=json.loads(small.stdout)
        for case in a.case or CASES:
            samples={'ours':[],'kactl':[]}
            # Alternate order; separate processes give equal cold static caches.
            for i in range(a.repeat):
                for provider in (('ours','kactl') if i%2==0 else ('kactl','ours')):
                    samples[provider].append(measured(exe,case,provider))
            row=dict(case=case,samples=samples)
            for provider,runs in samples.items():
                good=[r for r in runs if r.get('exit_code')==0]
                row[provider]=dict(all_passed=len(good)==len(runs))
                if good:
                    row[provider].update(median_ms=statistics.median(r['result']['algorithm_ms'] for r in good),
                                         peak_rss_mib=max(r['peak_rss_mib'] for r in good),
                                         checksums=sorted(set(str(r['result']['checksum']) for r in good)))
            row['equal_outputs']=(row['ours']['all_passed'] and row['kactl']['all_passed']
                                  and row['ours']['checksums']==row['kactl']['checksums'])
            if row['equal_outputs']:
                row['kactl_speedup']=row['ours']['median_ms']/row['kactl']['median_ms']
            report['results'].append(row);a.json_output.write_text(json.dumps(report,indent=2)+'\n')
            print(json.dumps({k:v for k,v in row.items() if k!='samples'}),flush=True)
    return 0 if all(r['equal_outputs'] for r in report['results']) else 1


if __name__=='__main__':raise SystemExit(main())
